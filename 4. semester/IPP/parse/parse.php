<?php
/**
 * @author Martin Fekete <xfeket00@stud.fit.vutbr.cz>
 * @date   09.03.2020
 */ 

// instruction counter
$instrCnt = 1;

// variables for extension
$commentCount = 0;
$jumpsCount = 0;
$uniqueLabels = array();

// DOM Document elements
$instruction;
$program;
$xml;

// arrays containing all valid opcodes
$allInstrArray = array("MOVE", "CREATEFRAME", "PUSHFRAME", "POPFRAME", "DEFVAR", "CALL", "RETURN", 
                       "PUSHS", "POPS", "ADD", "SUB", "MUL", "IDIV", "LT", "GT", "EQ", "AND", "OR", 
                       "NOT", "INT2CHAR", "STRI2INT", "READ", "WRITE", "CONCAT", "STRLEN", "GETCHAR", 
                       "SETCHAR", "TYPE", "LABEL", "JUMP", "JUMPIFEQ", "JUMPIFNEQ", "EXIT", "DPRINT", "BREAK");
// opcodes divided to groups by number of operands
$zeroArgArray = array("BREAK", "RETURN", "CREATEFRAME", "PUSHFRAME", "POPFRAME");
$oneArgArray = array("DEFVAR", "CALL", "PUSHS", "POPS", "WRITE", "LABEL", "JUMP", "EXIT", "DPRINT");
$twoArgsArray = array("MOVE", "INT2CHAR", "READ", "STRLEN", "TYPE", "NOT");
$threeArgsArray = array("ADD", "SUB", "MUL", "IDIV", "LT", "GT", "EQ", "AND", "OR", 
                        "STRI2INT", "CONCAT", "GETCHAR", "SETCHAR", "JUMPIFEQ", "JUMPIFNEQ");


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
 * Finds and removes comments from line, increments global comment counter
 * @param line one line from source code where comment will be deleted
 */
function removeComments($line) {
    global $commentCount;
    $line = trim(preg_replace("/#.*$/", "", $line, -1, $found));
    if ($found){
        $commentCount++;
    }
    return $line;
}

/**
 * Replaces invalid characters in string
 * @param oldStr string where characters are replaced
 */
function replaceCharacters($oldStr) {
    $newStr = str_replace("&", "&amp;", $oldStr);
    $newStr = str_replace('"', "&quot;", $newStr);
    $newStr = str_replace("'", "&apos;", $newStr);
    $newStr = str_replace("<", "&lt;", $newStr);
    $newStr = str_replace(">", "&gt;", $newStr);
    return $newStr;
}

/**
 * Checks validity of variable using regex
 * @param instr instruction
 */
function checkVar($instr) {
    if (!preg_match("/(*UTF8)^(LF|GF|TF)@[\p{L}_\-$&%*!?]+[\p{L}\p{N}_\-$&%*!?]*$/", $instr)) {
        printExit("Syntax or lexical error\n", 23);
    }
}

/**
 * Checks validity of label using regex
 * @param instr instruction
 */
function checkLabel($instr) {
    if (!preg_match("/(*UTF8)^[a-zA-Z_\-$&%*!?]+[\w_\-$&%*!?]*$/", $instr)) {
        printExit("Syntax or lexical error\n", 23);
    }
}

/**
 * Checks validity of symbol using regex
 * @param instr instruction
 */
function checkSymb($instr) {
    // looks terrible but serves its purpose
    if (!preg_match("/(*UTF8)^(LF|GF|TF)@[\p{L}_\-$&%*!?]+[\p{L}\p{N}_\-$&%*!?]*$/", $instr) &&
    !preg_match("/^(int@)([+-]?[1-9]\d*|0)$/", $instr) &&
    !preg_match("/^(string@)(\\\\\d{3,}|[^\\\\\s])*$/", $instr)  &&
    !preg_match("/^(bool@)(true|false)|(nil@nil)$/", $instr)) {
        printExit("Syntax or lexical error\n", 23);
    }
    return constOrVar($instr);
}

/**
 * Checks validity of type using regex
 * @param instr instruction
 */
function checkType($instr) {
    if (!preg_match("/^(nil|int|bool|string)$/", $instr)) {
        printExit("Syntax or lexical error\n", 23);
    }
}

/**
 * Decides whether operand in <symb> is constant or variable
 * @param instr instruction
 */
function constOrVar($instr) {
    if (preg_match("/(*UTF8)^(LF|GF|TF)@[\p{L}_\-$&%*!?]+[\p{L}\p{N}_\-$&%*!?]*$/", $instr)){
        return array("var", $instr);
    } else {
        $const = explode("@", $instr);
        return array($const[0], $const[1]);
    }
}

/**
 * Line contains instruciton with no operands
 * @param instr instruction
 */
function zeroArg($instr) {
    global $jumpsCount;
    $upperOpcode = strtoupper($instr[0]);
    if ($upperOpcode == "RETURN") {
        $jumpsCount++;
    }

    // OPCODE
    zeroArgXML($upperOpcode);
}

/**
 * Rewrites opcode with no operands to XML format
 * @param opcode opcode
 */
function zeroArgXML($opcode) {
    global $xml, $program, $instruction, $instrCnt;
    // new instruction
    $instruction = $xml->createElement("instruction");
    $instruction->setAttribute("order", $instrCnt);
    $instruction->setAttribute("opcode", $opcode);
    // append instruction to XML
    $program->appendChild($instruction);
    $instrCnt++;
}

/**
 * Line contains opcode with one operand
 * @param instr instruction
 */
function oneArg($instr) {
    global $jumpsCount, $uniqueLabels;
    $upperOpcode = strtoupper($instr[0]);
    $replacedStr = replaceCharacters($instr[1]);

    // OPCODE <var>
    if ($upperOpcode == "DEFVAR" || $upperOpcode == "POPS") {
        checkVar($instr[1]);
        oneArgXML($upperOpcode, "var", $replacedStr);
    // OPCODE <label>
    } else if ($upperOpcode == "CALL" || $upperOpcode == "LABEL" || $upperOpcode == "JUMP") {
        if ($upperOpcode == "LABEL") {
            if (!in_array($replacedStr, $uniqueLabels)) {
                array_push($uniqueLabels, $replacedStr);
            }
        } else if ($upperOpcode == "JUMP" || $upperOpcode == "CALL") {
            $jumpsCount++;
        }
        checkLabel($instr[1]);
        oneArgXML($upperOpcode, "label", $replacedStr);
    // OPCODE <symb>
    } else if ($upperOpcode == "PUSHS" || $upperOpcode == "WRITE" || $upperOpcode == "EXIT" || $upperOpcode == "DPRINT") {
        oneArgXML($upperOpcode, checkSymb($instr[1])[0], replaceCharacters(checkSymb($instr[1])[1]));
    }
}

/**
 * Rewrites opcode with one operand to XML format
 * @param opcode opcode
 * @param type1 type of opcode
 * @param arg1 argument of opcode
 */
function oneArgXML($opcode, $type1, $arg1) {
    global $xml, $program, $instruction, $instrCnt;
    // new instruction
    $instruction = $xml->createElement("instruction");
    $instruction->setAttribute("order", $instrCnt);
    $instruction->setAttribute("opcode", $opcode);
    $argXML = $xml->createElement("arg1", $arg1);
    $argXML->setAttribute("type", $type1);
    // append instruction and argument to XML
    $program->appendChild($instruction); 
    $instruction->appendChild($argXML);
    $instrCnt++;
}

/**
 * Line contains opcode with two operands
 * @param instr instruction
 */
function twoArgs($instr) {
    global $allInstrArray, $twoArgsArray;
    $upperOpcode = strtoupper($instr[0]);

    // OPCODE <var> <symb>
    if ($upperOpcode == "MOVE" || $upperOpcode == "INT2CHAR" || $upperOpcode == "STRLEN" || $upperOpcode == "TYPE" || $upperOpcode == "NOT") {
        checkVar($instr[1]);
        twoArgsXML($upperOpcode, "var", checkSymb($instr[2])[0], replaceCharacters($instr[1]), replaceCharacters(checkSymb($instr[2])[1]));
    // OPCODE <var> <type>
    } else if ($upperOpcode == "READ") {
        checkVar($instr[1]);
        checkType($instr[2]);
        twoArgsXML($upperOpcode, "var", "type", replaceCharacters($instr[1]), $instr[2]);
    }
}

/**
 * Rewrites opcode with one operand to XML format
 * @param opcode opcode
 * @param type1 type of opcode 1
 * @param type2 type of opcode 2
 * @param arg1 argument of opcode 1
 * @param arg2 argument of opcode 2
 */
function twoArgsXML($opcode, $type1, $type2, $arg1, $arg2) {
    global $xml, $program, $instruction, $instrCnt;
    // new instruction
    $instruction = $xml->createElement("instruction");
    $instruction->setAttribute("order", $instrCnt);
    $instruction->setAttribute("opcode", $opcode);
    $arg1XML = $xml->createElement("arg1", $arg1);
    $arg2XML = $xml->createElement("arg2", $arg2);
    $arg1XML->setAttribute("type", $type1);
    $arg2XML->setAttribute("type", $type2);
    // append instruction and argument to XML
    $program->appendChild($instruction); 
    $instruction->appendChild($arg1XML);
    $instruction->appendChild($arg2XML);
    $instrCnt++;
}

/**
 * Line contains opcode with three operands
 * @param instr instruction
 */
function threeArgs($instr) {
    global $jumpsCount;
    $upperOpcode = strtoupper($instr[0]);

    // OPCODE <label> <symb1> <symb2>
    if ($upperOpcode == "JUMPIFEQ" || $upperOpcode == "JUMPIFNEQ") {
        $jumpsCount++;
        checkLabel($instr[1]);
        threeArgsXML($upperOpcode, "label", checkSymb($instr[2])[0], checkSymb($instr[3])[0], replaceCharacters($instr[1]), checkSymb($instr[2])[1], checkSymb($instr[3])[1]);
    // OPCODE <var> <symb1> <symb2>
    } else {
        checkVar($instr[1]);
        threeArgsXML($upperOpcode, "var", checkSymb($instr[2])[0], checkSymb($instr[3])[0], replaceCharacters($instr[1]), checkSymb($instr[2])[1], checkSymb($instr[3])[1]);
    }
}

/**
 * Rewrites opcode with one operand to XML format
 * @param opcode opcode
 * @param type1 type of opcode 1
 * @param type2 type of opcode 2
 * @param type3 type of opcode 3
 * @param arg1 argument of opcode 1
 * @param arg2 argument of opcode 2
 * @param arg3 argument of opcode 3
 */
function threeArgsXML($opcode, $type1, $type2, $type3, $arg1, $arg2, $arg3) {
    global $xml, $program, $instruction, $instrCnt;
    // new instruction
    $instruction = $xml->createElement("instruction");
    $instruction->setAttribute("order", $instrCnt);
    $instruction->setAttribute("opcode", $opcode);
    $arg1XML = $xml->createElement("arg1", $arg1);
    $arg2XML = $xml->createElement("arg2", $arg2);
    $arg3XML = $xml->createElement("arg3", $arg3);
    $arg1XML->setAttribute("type", $type1);
    $arg2XML->setAttribute("type", $type2);
    $arg3XML->setAttribute("type", $type3);
    // append instruction and argument to XML
    $program->appendChild($instruction); 
    $instruction->appendChild($arg1XML);
    $instruction->appendChild($arg2XML);
    $instruction->appendChild($arg3XML);
    $instrCnt++;
}

/**
 * Checks validity of opcodes, exits with exitCode if not valid
 * @param opcode opcode thas is checked
 * @param instrArray array of valid opcodes
 * @param exitCode exit code of program if not found in array
 */
function checkArray($opcode, $instrArray, $exitCode) {
    global $allInstrArray, $zeroArgArray, $oneArgArray, $twoArgsArray, $threeArgsArray;
    if (!in_array($opcode, $instrArray)) {
        printExit("Wrong use of opcode\n", $exitCode);
    }
}

/* Core function of the script */
function main() {
    global $commentCount;
    global $allInstrArray, $zeroArgArray, $oneArgArray, $twoArgsArray, $threeArgsArray;
    global $xml, $program, $instruction, $instrCnt;
    
    // read the first line and check if its not EOF
    if (!($line = fgets(STDIN))) {
        printExit("Wrong header\n", 21);
    }
    // delete comments if necessary
    $line = removeComments($line);
    while ($line == "") {
        $line = removeComments(fgets(STDIN));
    }
    //  check if the header is valid
    if (strtolower($line) != ".ippcode20") {
        printExit("Wrong header\n", 21);
    }

    // we want to have required XML header and root element program at the beginnig of a XML
    $xml = new DomDocument('1.0', 'UTF-8');
    $xml->formatOutput = true;
    $program = $xml->createElement("program");
    $program->setAttribute("language", "IPPcode20");
    $xml->appendChild($program);

    // read each line of the file
    while ($line = fgets(STDIN)) {
        // check for comment
        $line = removeComments($line);
        // line is just a comment, skip it
        if ($line == "") {
            continue;
        }
        
        // replace all whitespace characters with single space, divide line to array
        $line = preg_replace("/\s+/", " ", $line);
        $lineArr = explode(" ", $line);
        $lineArr[0] = strtoupper($lineArr[0]);

        // check if the first word is OPCODE, if not, exit program with error message
        checkArray($lineArr[0], $allInstrArray, 22);
        
        // check type of instruction and its validity
        switch (count($lineArr)) {
            case 1:
                checkArray($lineArr[0], $zeroArgArray, 23);
                zeroArg($lineArr);
                break;
            case 2:
                checkArray($lineArr[0], $oneArgArray, 23);
                oneArg($lineArr);
                break;
            case 3:
                checkArray($lineArr[0], $twoArgsArray, 23);
                twoArgs($lineArr);
                break;
            case 4:
                checkArray($lineArr[0], $threeArgsArray, 23);
                threeArgs($lineArr);
                break;
            default:
                printExit("Syntax error.\n", 22);
        }
    }
    // print XML file to stdout
    echo $xml->saveXML();
}

// switch --help was used
if (in_array("--help", $argv)) {
    if ($argc == 2){
        printExit("The program expects IPPcode20 on STDIN, outputs XML version of IPPcode20.\n", 0);
    } else {
        printExit("--help should not be used with other arguments.\n", 10);
    }
}

// no switch used
if ($argc == 1) {
    main();
    exit(0);
// switches other than --help were used
} else {
    $file;
    $fileGiven = false;
    $validSwitch = array("$argv[0]", "--loc", "--comments", "--labels", "--jumps", "--help");

    foreach ($argv as $value) {
        // finds file for stats, if not found, bool fileGiven remains false and program will end later
        if (preg_match("/^--stats=.+$/", $value)) {
            $file = explode("=", $value, 2)[1];
            $fileGiven = true;
        }
        // checks if switch used is valid
        if (!in_array($value, $validSwitch) && !preg_match("/^--stats=.+$/", $value)) {
            printExit("Wrong switch used\n", 10);
        }
    }
    
    // no stats file was given
    if (!$fileGiven) {
        printExit("No stats file given\n", 10);
    }

    // open file, print error message if not successful
    $file = fopen($file, "w");
	if (!$file) {
	    printExit("File cannot be opened\n", 12);
    }

    main();

    // instruction are indexed from 1, not zero => need to decrement
    $instrCnt--;
    // write stats to given file
    foreach ($argv as $value) {
        switch ($value) {
            case '--comments':
                fwrite($file, "$commentCount\n");
                break;
            case '--loc':
                fwrite($file, "$instrCnt\n");
                break;
            case '--jumps':
                fwrite($file, "$jumpsCount\n");
                break;
            case '--labels':
                fwrite($file, count($uniqueLabels) . "\n");
                break;
        }
    }
    exit(0);
}

?>