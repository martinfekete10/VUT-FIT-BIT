<?php
/**
 * @author Martin Fekete <xfeket00@stud.fit.vutbr.cz>
 * @date   30.03.2020
 */ 

// test counter
$testNo = 0;
$passed = 0;
$failed = 0;
$passedDir = 0;
$failedDir = 0;

// switch variables
$recursive = false;
$parseOnlyBool = false;
$intOnlyBool =  false;

// paths to files
$dir = array(getcwd());
$parseScript = getcwd()."/parse.php";
$intScript = getcwd()."/interpret.py";
$jexamxml = "/pub/courses/ipp/jexamxml/jexamxml.jar";

// FILES
$testFile = false;
$matchRegex = false;
$src_list = array();

/**
 * Prints message to STDERR and exits program with given exit code 
 * @param message  message sent to stderr
 * @param exitCode code script ends with
 */ 
function printExit($message, $exitCode){
    fprintf(STDERR, $message);
    exit($exitCode);
}

/**
 * Returns an array with the list of sub directories of dir
 * @param dir directory which will be resursively searched for subdirectories
 */
function recursiveDirectories($dir) {
    $subDir = array();
    $directories = array_filter(glob($dir), "is_dir");
    $subDir = array_merge($subDir, $directories);
    foreach ($directories as $directory) {
        $subDir = array_merge($subDir, recursiveDirectories($directory."/*"));
    }
    return $subDir;
}

/**
 * Generates HTML header style classes
 */
function generateBeginning() {
    echo "<!DOCTYPE html>\n";
    echo "<html>\n";
    echo "<head>\n";
    
    echo "<style>\n";
    echo ".header {\npadding: 10px;\ntext-align: center;\nfont-family: courier;\nbackground: #0075B8;\ncolor: white;\nfont-size: 20px;\n}\n";
    echo ".content {\npadding:30px;\ntext-align: center;\nfont-family: courier;\nbackground: rgb(240, 240, 240);\n}\n";
    echo "</style>\n";
    
    echo "<title>IPP test.php</title>\n";
    echo "<meta charset='utf-8'>\n";
    echo "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    echo "</head>\n";
    
    echo "<body>\n";
    echo "<div class=\"header\">\n";
    echo "<h1>IPP test script</h1>\n";
    echo "<h5>Martin Fekete | xfeket00@stud.fit.vutbr.cz</h5>\n";
    echo "</div>\n";

    echo "<div class=\"content\">\n";
}

/**
 * Generates passed test to HTML
 * @param testName name of the passed test
 */
function generatePass($testName) {
    echo "<h3 style=\"color:green;\">$testName: PASS</h3>\n";
}

/**
 * Generates failed test to HTML
 * @param testName name of the failed test
 */
function generateFail($testName) {
    echo "<h3 style=\"color:red;\">$testName: FAIL</h3>\n";
}

/**
 * Generates information about folder where tests are to HTML
 * @param dir name of the directory
 */
function generateFolderBeginning($dir) {
    echo "<p>$dir</p>\n";
}

/**
 * Generates stats about tests ran in the folder
 * @param totalDir  total number of tests in directory
 * @param passedDir number of passed tests in direcotry
 * @param failedDir number of failed tests in direcotry
 */
function generateFolderEnd($totalDir, $passedDir, $failedDir) {
    echo "<br>\n";
    if ($totalDir == $passedDir) {
        echo "<h3 style=\"color:green;\">All $passedDir/$passedDir tests from directory passed!</h3>\n";
    } else {
        echo "<h3 style=\"color:green;\">Passed: $passedDir</h3>\n";
        echo "<h3 style=\"color:red;\">Failed: $failedDir</h3>\n";
    }
    echo "<hr>\n";
}

/**
 * Generates message that no .src file were found in current folder
 */
function generateNoSRC() {
    echo "<br>\n";
    echo "<h3>No .src files found in this folder</h3>\n";
    echo "<hr>\n";     
}

/**
 * Generates closing HTML tags at the end of file
 * @param total  total number of tests
 * @param passed number of passed tests
 * @param failed number of failed tests
 */
function generateEnd($total, $passed, $failed) {
    global $testNo, $passed, $failed;
    echo "<hr>\n";
    if ($total == 0) {
        echo "<h3>No .src files to test found</h3>\n";   
    } else if ($testNo == $passed) {
        echo "<h3 style=\"color:green;\">All $testNo/$testNo tests passed!</h3>\n";
    } else {
        $stat = number_format(($passed / $total)*100, 2, '.', '');
        echo "<h2>$stat% PASSED</h2>\n";
        echo "<br>\n";
        echo "<h3>TOTAL: $testNo</h3>\n";
        echo "<h3 style=\"color:green;\">Passed: $passed</h3>\n";
        echo "<h3 style=\"color:red;\">Failed: $failed</h3>\n";
    }
    echo "</div>\n";
    echo "</body>\n";
}


/**
 * Saves all paths to dirsectories and .src files from testlist file to variable $dir and $src_list
 * @param file file with paths
 */
function testlist($file) {
    global $recursive;

    $dirs = array();
    $src_list = array();
    $testFile = @fopen($file, "r") or printExit("Could not open test list file\n", 11);

    while (!feof($testFile)) {
        
        $line = fgets($testFile);
        $line = preg_replace("/\n$/", "", $line);
        $line = preg_replace("/\/$/", "", $line);
    
        if ($line == "") {
            continue;
        }

        if (is_dir($line)) {
            if ($recursive) {
                $dirs = array_merge($dirs, recursiveDirectories($line));
            } else {
                array_push($dirs, $line);
            }
        } else if (preg_match("/\.src$/", $line)) {
            if (!is_file($line)) {
                printExit("Directory path in switch --testlist=file not valid\n", 11);
            }
            array_push($src_list, $line);
        } else {
            printExit("Line in testlist file is neither directory nor file\n", 11);
        }
    }    
    fclose($testFile);

    return array($dirs, $src_list);
}

/**
 * --parse-only test
 * @param dir array of directories there tests will be searched
 */
function parseOnly($dir) {
    global $testNo, $passed, $failed, $passedDir, $failedDir;
    global $parseScript, $jexamxml, $src_list, $matchRegex;

    if ($dir == "src") {
        echo "<p>Testing files from testlist file</p>\n";
        $srcFiles = $src_list;
    } else {
        generateFolderBeginning($dir);
        $srcFiles = $dir."/*.src";
        $srcFiles = glob($srcFiles, GLOB_BRACE);
    }
    sort($srcFiles);

    // there are no .src files in the folder
    if (empty($srcFiles)) {
        generateNoSRC();
        return;
    }

    foreach ($srcFiles as $srcFilePath) {
        if ($matchRegex) {
            $name = substr(basename($srcFilePath), 0, -4);
            if (!preg_match($matchRegex, $name)) {
                continue;
            }
        }

        $testNo++;
        $totalDir++;
        
        // execute
        $command = "php7.4 $parseScript < $srcFilePath > xfeket00-test";
        @exec($command, $output, $retVal);

        // load .rc file to variable, if it does not exist, we assume that return code is 0 and create it
        $rcFilePath = substr_replace($srcFilePath , "rc", strrpos($srcFilePath , '.') + 1);
        if (file_exists($rcFilePath) && !is_readable($rcFilePath)) {
            printExit("No permission to read .rc file\n", 11);
        }
        if (($rcContents = @file_get_contents($rcFilePath, "r")) === false) {
            $rcFile = fopen($rcFilePath, "w");
            fwrite($rcFile, "0");
            fclose($rcFile);
            $rcContents = 0;
        }
        
        // load .out file to variable, if it does not exist, create an empty one
        $outFilePath = substr_replace($srcFilePath , "out", strrpos($srcFilePath , '.') + 1);
        if (file_exists($outFilePath) && !is_readable($outFilePath)) {
            printExit("No permission to read .out file\n", 11);
        }
        if (($outContents = @file_get_contents($outFilePath, "r")) === false) {
            $outFile = fopen($outFilePath, "w");
            fclose($outFile);
        }

        // create .in file
        $inFilePath = substr_replace($srcFilePath , "in", strrpos($srcFilePath , '.') + 1);
        if (file_exists($inFilePath) && !is_readable($inFilePath)) {
            printExit("No permission to read .in file\n", 11);
        }
        if (($inContents = @file_get_contents($inFilePath, "r")) === false) {
            $inFile = fopen($inFilePath, "w");
            fclose($inFile);
        }
        
        // src file contains lexical or syntactic error
        if ($retVal != 0) {
            if ($rcContents == $retVal) {
                $passed++;
                $passedDir++;
                generatePass(basename($srcFilePath));
            } else {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
            }
        // src file is ok
        } else {
            
            // compare real return code and .rc return code
            if ($rcContents != $retVal) {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
                continue;
            }
            
            @exec("java -jar $jexamxml xfeket00-test $outFilePath", $output, $retVal);
            
            if ($retVal == 0) {
                $passed++;
                $passedDir++;
                generatePass(basename($srcFilePath));
            } else {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
            }
        }
    }

    generateFolderEnd($totalDir, $passedDir, $failedDir);
    $totalDir = $passedDir = $failedDir = 0;

    // delete temporary files
    $first = getcwd()."/xfeket00-test";
    if (file_exists($first)) {
        @exec("rm " . $first);
    }
    $second = getcwd()."/xfeket00-test.log";
    if (file_exists($second)) {
        @exec("rm " . $second);
    }
}

/**
 * --int-only test
 * @param dir array of directories there tests will be searched
 */
function intOnly($dir) {
    global $testNo, $passed, $failed, $passedDir, $failedDir;
    global $intScript, $jexamxml, $src_list, $matchRegex;

    if ($dir == "src") {
        echo "<p>Testing files from testlist file</p>\n";
        $srcFiles = $src_list;
    } else {
        generateFolderBeginning($dir);
        $srcFiles = $dir."/*.src";
        $srcFiles = glob($srcFiles, GLOB_BRACE);
    }
    sort($srcFiles);

    // there are no .src files in the folder
    if (empty($srcFiles)) {
        generateNoSRC();
        return;
    }

    foreach ($srcFiles as $srcFilePath) {
        if ($matchRegex) {
            $name = substr(basename($srcFilePath), 0, -4);
            if (!preg_match($matchRegex, $name)) {
                continue;
            }
        }

        $testNo++;
        $totalDir++;

        // load .in file to variable
        $inFilePath = substr_replace($srcFilePath , "in", strrpos($srcFilePath , '.') + 1);
        if (file_exists($inFilePath) && !is_readable($inFilePath)) {
            printExit("No permission to read .in file\n", 11);
        }
        if (($inContents = @file_get_contents($inFilePath, "r")) === false) {
            $inFile = fopen($inFilePath, "w");
            fclose($inFile);
        }
        
        // execute interpret
        $command = "python3.8 $intScript --source=$srcFilePath --input=$inFilePath > xfeket00-test";        
        @exec($command, $output, $retVal);

        // load .rc file to variable, if it does not exist, we assume that return code is 0 and create it
        $rcFilePath = substr_replace($srcFilePath , "rc", strrpos($srcFilePath , '.') + 1);
        if (file_exists($rcFilePath) && !is_readable($rcFilePath)) {
            printExit("No permission to read .rc file\n", 11);
        }
        if (($rcContents = @file_get_contents($rcFilePath, "r")) === false) {
            $rcFile = fopen($rcFilePath, "w");
            fwrite($rcFile, "0");
            fclose($rcFile);
            $rcContents = 0;
        }
        
        // load .out file to variable, if it does not exist, create an empty one
        $outFilePath = substr_replace($srcFilePath , "out", strrpos($srcFilePath , '.') + 1);
        if (file_exists($outFilePath) && !is_readable($outFilePath)) {
            printExit("No permission to read .out file\n", 11);
        }
        if (($outContents = @file_get_contents($outFilePath, "r")) === false) {
            $outFile = fopen($outFilePath, "w");
            fclose($outFile);
        }

        // src file contains error
        if ($retVal != 0) {
            if ($rcContents == $retVal) {
                $passed++;
                $passedDir++;
                generatePass(basename($srcFilePath));
            } else {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
            }
        // return value is 0
        } else {
            
            // compare real return code and .rc return code
            if ($rcContents != $retVal) {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
                continue;
            }

            @exec("diff xfeket00-test $outFilePath", $output, $retVal);

            if ($retVal == 0) {
                $passed++;
                $passedDir++;
                generatePass(basename($srcFilePath));
            } else {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
            }
        }
    }

    generateFolderEnd($totalDir, $passedDir, $failedDir);
    $totalDir = $passedDir = $failedDir = 0;

    // delete temporary files
    $first = getcwd()."/xfeket00-test";
    if (file_exists($first)) {
        @exec("rm " . $first);
    }
    $second = getcwd()."/xfeket00-test.log";
    if (file_exists($second)) {
        @exec("rm " . $second);
    }
}

/**
 * parse.php and interpret.py test
 * @param dir array of directories there tests will be searched
 */
function both($dir) {
    global $testNo, $passed, $failed, $passedDir, $failedDir;
    global $intScript, $parseScript, $jexamxml, $src_list, $matchRegex;

    if ($dir == "src") {
        echo "<p>Testing files from testlist file</p>\n";
        $srcFiles = $src_list;
    } else {
        generateFolderBeginning($dir);
        $srcFiles = $dir."/*.src";
        $srcFiles = glob($srcFiles, GLOB_BRACE);
    }
    sort($srcFiles);


    // there are no .src files in the folder
    if (empty($srcFiles)) {
        generateNoSRC();
        return;
    }

    foreach ($srcFiles as $srcFilePath) {
        if ($matchRegex) {
            $name = substr(basename($srcFilePath), 0, -4);
            if (!preg_match($matchRegex, $name)) {
                continue;
            }
        }

        $testNo++;
        $totalDir++;
        
        // execute parser
        $command = "php7.4 $parseScript < $srcFilePath > xfeket00-test.xml";
        @exec($command, $output, $retVal);

        // load .rc file to variable, if it does not exist, assume that return code is 0 and create it
        $rcFilePath = substr_replace($srcFilePath , "rc", strrpos($srcFilePath , '.') + 1);
        if (file_exists($rcFilePath) && !is_readable($rcFilePath)) {
            printExit("No permission to read .rc file\n", 11);
        }
        if (($rcContents = @file_get_contents($rcFilePath, "r")) === false) {
            $rcFile = fopen($rcFilePath, "w");
            fwrite($rcFile, "0");
            fclose($rcFile);
            $rcContents = 0;
        }
        
        // load .in file to variable
        $inFilePath = substr_replace($srcFilePath , "in", strrpos($srcFilePath , '.') + 1);
        if (file_exists($inFilePath) && !is_readable($inFilePath)) {
            printExit("No permission to read .in file\n", 11);
        }
        if (($inContents = @file_get_contents($inFilePath, "r")) === false) {
            $inFile = fopen($inFilePath, "w");
            fclose($inFile);
        }
        
        // load .out file to variable, if it does not exist, create an empty one
        $outFilePath = substr_replace($srcFilePath , "out", strrpos($srcFilePath , '.') + 1);
        if (file_exists($outFilePath) && !is_readable($outFilePath)) {
            printExit("No permission to read .out file\n", 11);
        }
        if (($outContents = @file_get_contents($outFilePath, "r")) === false) {
            $outFile = fopen($outFilePath, "w");
            fclose($outFile);
        }

        // error during parsing (syntactic or lexical) -> compare just return values
        if ($retVal != 0) {
            if ($rcContents == $retVal) {
                $passed++;
                $passedDir++;
                generatePass(basename($srcFilePath));
                continue;
            } else {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
                continue;
            }
        }

        // execute interpret
        $command = "python3.8 $intScript --source=xfeket00-test.xml --input=$inFilePath > xfeket00-test";
        @exec($command, $output, $retVal);

        // error during interpretation (semantic) -> compare just return values
        if ($retVal != 0) {
            if ($rcContents == $retVal) {
                $passed++;
                $passedDir++;
                generatePass(basename($srcFilePath));
            } else {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
            }
        // return value is 0
        } else {

            // compare real return code and .rc return code
            if ($rcContents != $retVal) {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
                continue;
            }

            @exec("diff xfeket00-test $outFilePath", $output, $retVal);

            if ($retVal == 0) {
                $passed++;
                $passedDir++;
                generatePass(basename($srcFilePath));
            } else {
                $failed++;
                $failedDir++;
                generateFail(basename($srcFilePath));
            }
        }
    }

    generateFolderEnd($totalDir, $passedDir, $failedDir);
    $totalDir = $passedDir = $failedDir = 0;

    // delete temporary files
    $first = getcwd()."/xfeket00-test";
    if (file_exists($first)) {
        @exec("rm " . $first);
    }
    $second = getcwd()."/xfeket00-test.log";
    if (file_exists($second)) {
        @exec("rm " . $second);
    }
    $third = getcwd()."/xfeket00-test.xml";
    if (file_exists($third)) {
        @exec("rm " . $third);
    }
}

// SWITCHES
$validSwitch = array("$argv[0]", "--help", "--recursive", "--parse-only", "--int-only");

// --help
if (in_array("--help", $argv)) {
    if ($argc == 2){
        printExit("The script tests interpret.py and parse.php\n", 0);
    } else {
        printExit("--help should not be used with other arguments\n", 10);
    }
}

// other switches
foreach ($argv as $value) {
    // check if switch is valid
    if (!in_array($value, $validSwitch) 
        && !preg_match("/^--directory=.+$/", $value)
        && !preg_match("/^--parse-script=.+$/", $value) 
        && !preg_match("/^--int-script=.+$/", $value)
        && !preg_match("/^--jexamxml=.+$/", $value)
        && !preg_match("/^--testlist=.+$/", $value)
        && !preg_match("/^--match=.+$/", $value)) {
        printExit("Wrong switch used\n", 10);
    }
    // switch is valid
    if ($value == "--parse-only"){
        $parseOnlyBool = true;
    } else if ($value == "--int-only") {
        $intOnlyBool = true;
    } else if ($value == "--recursive") {
        $recursive = true;
    } else if (preg_match("/^--directory=.+$/", $value)) {
        $dir[0] = explode("=", $value)[1];
        $dir[0] = preg_replace("/\/$/", "", $dir[0]);
    } else if (preg_match("/^--parse-script=.+$/", $value)) {
        $parseScript = explode("=", $value)[1];
    } else if (preg_match("/^--int-script=.+$/", $value)) {
        $intScript = explode("=", $value)[1];
    } else if (preg_match("/^--jexamxml=.+$/", $value)) {
        $jexamxml = explode("=", $value)[1];
    } else if (preg_match("/^--testlist=.+$/", $value)) {
        $testFile = explode("=", $value)[1];
    } else if (preg_match("/^--match=.+$/", $value)) {
        $matchRegex = explode("=", $value)[1];
        $matchRegex = "/" . $matchRegex . "/";
        if (@preg_match($matchRegex, "Test of regex") === false) {
            printExit("Syntax in regular expression", 11);
        }
    }
}

// --int-only and --parse-only used together
if ($intOnlyBool && $parseOnlyBool) {
    printExit("Switches --int-only and --parse-only cannot be used together\n", 10);
}

// check validity of path
if (!is_dir($dir[0])) {
    printExit("Directory path in switch --directory=path is not valid\n", 11);
}

// --recursive switch used
if ($recursive && !$testFile) {
    $dir = recursiveDirectories($dir[0]);
}

// check if parse script exists in working directory / given directory
if (!file_exists($parseScript) || !preg_match("/^.*\.php$/", $parseScript)) {
    printExit("Given parse script file is not valid\n", 11);
}

// check if interpret script exists in working directory / given directory
if (!file_exists($intScript) || !preg_match("/^.*\.py$/", $intScript)) {
    printExit("Given interpret script file is not valid\n", 11);
}

// check if jexamxml file exists in working directory / given directory
if (!file_exists($jexamxml) || !preg_match("/^.*\.jar$/", $jexamxml)) {
    printExit("Given tool is not valid\n", 11);
}

// testfile bonus
if ($testFile) {
    // --testlist=file and --direcotry used together
    if (in_array("/^--directory=.+$/", $argv)) {
        printExit("--testlist and --directory flags used together", 10);
    }

    list($dir, $src_list) = testlist($testFile);
}

// change all paths to full paths
foreach($dir as $i => $dirPath) {
    $dir[$i] = realpath($dir[$i]);
}

generateBeginning();

// parse-only
if ($parseOnlyBool) {
    foreach ($dir as $dir) {
        parseOnly($dir);
    }
    if (!empty($src_list)) {
        parseOnly("src");
    }
}

// int-only
if ($intOnlyBool) {
    foreach ($dir as $dir) {
        intOnly($dir);
    }
    if (!empty($src_list)) {
        intOnly("src");
    }
}

// both
if (!$parseOnlyBool && !$intOnlyBool) {
    foreach ($dir as $dir) {
        both($dir);
    }
    if (!empty($src_list)) {
        both("src");
    }
}

generateEnd($testNo, $passed, $failed);

?>