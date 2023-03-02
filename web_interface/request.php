<?php
// error_reporting(E_ALL);
// ini_set('display_errors', '1');

header('Content-type: application/json');

if ($_SERVER['REQUEST_METHOD'] != 'POST') {
    http_response_code(400);
    echo('{"error":"not a post request"}');
    exit();
}

$request_raw = file_get_contents('php://input');
if ($request_raw == "") {
    http_response_code(400);
    echo('{"error":"request body missing"}');
    exit();
}
$input = json_decode($request_raw);
if ($input == NULL) {
    http_response_code(400);
    echo('{"error":"json decode fail"}');
    exit();
}

$descriptorspec = array(
    0 => array("pipe", "r"), // stdin
    1 => array("pipe", "w"), // stdout
    2 => array("pipe", "w"), // stderr
    //TODO can also be "file", "thefilepath", "writeperms"
);
// calling script with timeout
$timeout = 5;
$process = proc_open("timeout $timeout ./cedh_matching --timeout 2000", $descriptorspec, $pipes);

$output = "";
if (is_resource($process)) {
    fwrite($pipes[0], json_encode($input)); // sending stdin input
    fclose($pipes[0]);

    $error_out = "";
    while (!feof($pipes[2])) {
        $error_line = fgets($pipes[2]);
        if ($error_line == false) {
            break;
        }
        $error_out .= $error_line;
    }
    if ($error_out != "") {
        $error_out = str_replace("\n", "\\n", $error_out);
        $output = '{"error":"' . addslashes($error_out) . '"}';
    }
    
    if ($error_out == "") {
        while (!feof($pipes[1])) {
            $output_line = fgets($pipes[1]);
            if ($output_line == false) {
                break;
            }
            $output .= $output_line; //getting output of the script
        }
    }
} else {
    http_response_code(400);
    $output = '{"error":"could not open resource"}';
}

proc_close($process);

echo($output);
?>
