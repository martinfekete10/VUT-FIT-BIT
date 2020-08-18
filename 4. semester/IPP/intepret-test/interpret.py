# ---------------------------------------------------
# Author: Martin Fekete <xfeket00@stud.fit.vutbr.cz>
# Date:   04.04.2020
# ---------------------------------------------------

import sys, re
import xml.etree.ElementTree as et

# ------------------
# Prints exit message and exits script with err_code
# ------------------
def print_exit(message, err_code):
    sys.stderr.write(message + "\n")
    sys.exit(err_code)

# ------------------
# Checks all script arguments
# ------------------
def parse_args():
    global source_bool, input_bool, input_file, source_file, stats_bool, stats_file

    vars_bool = False
    insts_bool = False
    
    # --help switch, must not be used with other switches
    if ("--help" in sys.argv):
        if (len(sys.argv) != 2):
            print_exit("Help should not be used with other arguments", 10)
        else:
            print("This program interprets XML representation of IPPcode20")
            print("Usage: python3.8 interpret.py --source=file --input=file")
            sys.exit(0)

    # Only possible switches are --source=+ and --input=+, check for them and save
    for arg in sys.argv[1:]:
        if re.match(r"--source=.+", arg):
            source_bool = True
            source_file = arg.split("=")[1]
        elif re.match(r"--input=.+", arg):
            input_bool = True
            input_file = arg.split("=")[1]
        elif re.match(r"--stats=.+", arg):
            stats_bool = True
            stats_file = arg.split("=")[1]
        elif re.match(r"--insts", arg):
            insts_bool = True
        elif re.match(r"--vars", arg):
            vars_bool = True
        else:
            print_exit("Used switch is not valid", 10)

    # Neither input nor source file given
    if (not input_bool) and (not source_bool):
        print_exit("Neither source nor input were given", 10)

    # No stats file given
    if (insts_bool or vars_bool) and (not stats_bool):
        print_exit("No stats file was given", 10)


# ------------------
# Write statistics to given file
# ------------------
def do_stats(stats_file, total_instr, max_vars):
    try:
        f = open(stats_file, "w")
    except Exception:
        print_exit("do_stats -> could not open file for stats", 12)
    for arg in sys.argv[1:]:
        if re.match(r"--insts", arg):
            f.write(str(total_instr) + "\n")
        elif re.match(r"--vars", arg):
            f.write(str(max_vars) + "\n")
    f.close()

# ------------------
# Saves source contents to variable
# Returns contents of source file
# ------------------
def read_source(source_bool, source_file):
    if source_bool:
        try:
            with open(source_file, 'r') as file:
                source_content = file.read()
        except Exception:
            print_exit("Could not open source file", 11)
    else:
        contents = []
        while True:
            try:
                line = input() + "\n"
                contents.append(line)
            except EOFError:
                break
        source_content = "".join(contents)
    
    return source_content


# ------------------
# Checks if of arg contains only one attribute
# ------------------
def check_arg(arg):
    if (len(arg.attrib) != 1):
        print_exit("XML instruction -> more attributes than expected", 32)


# ------------------
# Checks validity of instruction element
# ------------------
def check_instr(instr):
    if (instr.tag != "instruction"):
        print_exit("Wrong XML format -> instr.tag not instruction", 32)
    if (len(instr.attrib) != 2):
        print_exit("Wrong XML format -> instr.attrib - lenght", 32)
    if ("opcode" not in instr.attrib):
        print_exit("Wrong XML format -> instr.attrib - opcode", 32)
    if ("order" not in instr.attrib):
        print_exit("Wrong XML format -> instr.attrib - order", 32)
    if (int(instr.attrib["order"]) < 0):
        print_exit("Wrong XML format -> instr.attrib - order", 32)


# ------------------
# Checks semantic validity of constant
# Returns (converted) value
# ------------------
def check_return_const(arg):
    if (arg.attrib["type"] == "string"):
        if (arg.text) == None:
            arg.text = ""
        if not re.match(r"^(\\\d{3,}|[^\\\s])*$", arg.text):
            print_exit("Type specified as string not valid", 53)
        return convert_escape(arg.text)
    
    elif (arg.attrib["type"] == "int"):
        if (arg.text == None):
            print_exit("Int must not be empty", 32)
        if not re.match(r"^[+-]?[1-9]\d*|0$", arg.text):
            print_exit("Type specified as int not int", 53)
        return arg.text
    
    elif (arg.attrib["type"] == "bool"):
        if (arg.text == None):
            print_exit("Bool must not be empty", 32)
        if (arg.text != "true") and (arg.text != "false"):
            print_exit("Type specified as bool not bool", 53)
        else:
            return arg.text
    
    elif (arg.attrib["type"] == "nil"):
        if (arg.text == None):
            print_exit("Nil must not be empty", 32)
        if (arg.text != "nil"):
            print_exit("Type specified as nil not nil", 53)
        else:
            return ""

    elif (arg.attrib["type"] == "float"):
        if (arg.text == None):
            print_exit("Float must not be empty", 32)
        try:
            return float.fromhex(arg.text)
        except Exception:
            print_exit("Type specified as float not float", 53)
    
    else:
        print_exit("Specified type not known", 32)


# ------------------
#  Checks variable
# ------------------
def check_var(arg):
    if (arg.attrib["type"] != "var"):
        print_exit("check_var -> argument type not valid", 32)
    if not re.search(r"^(LF|TF|GF)@[\w_\-$&%*!?][\w\d_\-$&%*!?]*$", arg.text):
        print_exit("Variable definition not valid", 32)
    if re.search(r"(LF|TF|GF)@[\d]", arg.text):
        print_exit("Variable definition not valid", 32)


# ------------------
#  Checks label
# ------------------
def check_label(arg):
    if (arg.attrib["type"] != "label"):
        print_exit("check_label -> argument type not valid", 32)
    if not re.search(r"^[a-zA-Z_\-$&%*!?][\w_\-$&%*!?]*$", arg.text):
        print_exit("check_label -> label name not valid", 32)


# ------------------
#  Checks type
# ------------------
def check_type(arg):
    if (arg.attrib["type"] != "type"):
        print_exit("check_type -> argument type not valid", 32)
    if not re.search(r"^(int|bool|string|nil|float)$", arg.text):
        print_exit("check_type -> type not int / string / bool", 32)


# ------------------
#  Checks if label exists
# ------------------
def check_label_jump(arg):
    global labels
    if (arg.attrib["type"] != "label"):
            print_exit("JUMP -> wrong argument type", 32)
    if (arg.text not in labels):
        print_exit("JUMP -> jump to undefined label", 52)


# ------------------
# Converts escape sequencies to chars
# ------------------
def convert_escape(string):
    i = 0
    while (i < len(string)):
        if (string[i] == "\\"):
            escape = string[i:i+4]
            if (escape[1] == "0"):
                escape = escape[2:]
            if (escape[0] == "0"):
                escape = escape[1:]
            if (escape[0] == "0"):
                escape = escape[1:]
            escape = chr(int(escape))
            string = string.replace(string[i:i+4], escape)
        i += 1
    return string


# ------------------
# Checks if variable is defined
# Returns scope and name of var
# ------------------
def scope_name_check_var(arg):
    global GF, LF, TF, GF_type, LF_type, TF_type, TF_access
    check_var(arg)
    scope, name = arg.text.split("@")
    if (scope == "GF"):
        if name not in GF.keys():
            print_exit("scope_name_check_var -> GF variable undefined", 54)
    elif (scope == "LF"):
        if not (LF[-1]) and (len(LF) == 1):
            print_exit("scope_name_check_var -> LF frame not defined", 55)
        if name not in LF[-1].keys():
            print_exit("scope_name_check_var -> LF variable undefined", 54)
    else:
        if not (TF_access):
            print_exit("scope_name_check_var -> TF frame not defined", 55)
        if name not in TF.keys():
            print_exit("scope_name_check_var -> TF variable undefined", 54)
    return scope, name


# ------------------
# Checks if instruction argument is constant or variable
# Returns value and type of symb
# ------------------
def const_or_var(arg, TYPE = None):
    constants = ["string", "int", "bool", "nil", "float"]

    # <const>
    if (arg.attrib["type"] in constants):
        val = check_return_const(arg)
        typ = arg.attrib["type"]
        return val, typ
    
    # <var>
    elif (arg.attrib["type"] == "var"):
        scope, name = scope_name_check_var(arg)
        
        if scope == "GF":
            val = GF.get(name)
            typ = GF_type.get(name)
            if not TYPE:
                if (val == None):
                    print_exit("const_or_var -> unset val of varaible", 56)

        if scope == "LF":
            val = LF[-1].get(name)
            typ = LF_type[-1].get(name)
            if not TYPE:
                if (val == None):
                    print_exit("const_or_var -> unset val of varaible", 56)

        if scope == "TF":
            val = TF.get(name)
            typ = TF_type.get(name)
            if not TYPE:
                if (val == None):
                    print_exit("const_or_var -> unset val of varaible", 56)
    
    # err
    else:
        print_exit("const_or_var -> neither const nor var", 32)

    if (typ == "nil"):
        val = ""
    
    return val, typ


# ------------------
# Adds new value and type to variable "name" at given scope
# ------------------
def add_var_value(scope, name, val, typ):
    global GF, LF, TF, GF_type, LF_type, TF_type, TF_access, pushed
    
    if (scope == "GF"):
        GF[name] = val
        GF_type[name] = typ
    elif (scope == "LF"):
        if not (pushed[-1]):
            print_exit("add_var_value -> LF frame not accessible", 55)
        LF[-1][name] = val
        LF_type[-1][name] = typ
    elif (scope == "TF"):
        if not (TF_access):
            print_exit("add_var_value -> TF frame not accessible", 55)
        TF[name] = val
        TF_type[name] = typ


# ------------------
# Count maximum number of initialized variables in all scopes
# Returns either current max or new max of variables
# ------------------
def count_vars(max_vars):
    global GF, LF, TF, TF_access, pushed
    
    tmp_max = 0

    # GF
    for val in GF.values():
        if (val != None):
            tmp_max += 1
    # LF
    for i in range(len(LF)):
        for val in LF[i].values():
            if (val != None):
                tmp_max += 1
    # TF
    if (TF_access):
        for val in TF.values():
            if (val != None):
                tmp_max += 1
    
    if tmp_max > max_vars:
        return tmp_max
    else:
        return max_vars


# ------------------
# Chcecks if variable at given scope already exists
# ------------------
def check_var_redef(scope, name):
    global GF, LF, TF, GF_type, LF_type, TF_type

    if (scope == "GF"):
        if name in GF.keys():
            print_exit("DEFVAR -> GF variable redefinition", 52)
    elif (scope == "LF"):
        if name in LF[-1].keys():
            print_exit("DEFVAR -> LF variable redefinition", 52)
    elif (scope == "TF"):
        if name in TF.keys():
            print_exit("DEFVAR -> TF variable redefinition", 52)


# ------------------
# Iterates through all arguments of instruction and saves them to arrays
# Returns scope of variable <var>, name of variable <var>, values and types of <symb1> and <symb2>
# ------------------
def iterate_var_symb(instr, iter_no):
    val = []
    typ = []

    i = 1
    for arg in instr:
        
        # For instructions with only 2 operands
        if (i > iter_no):
            break
        
        check_arg(arg)

        # <var>
        if (i == 1):
            scope, name = scope_name_check_var(arg)

        # <symb1>
        if (i == 2):
            val_tmp, typ_tmp = const_or_var(arg)
            val.insert(i-2, val_tmp)
            typ.insert(i-2, typ_tmp)

        # <symb2> -- optional
        if (i == 3):
            val_tmp, typ_tmp = const_or_var(arg)
            val.insert(i-2, val_tmp)
            typ.insert(i-2, typ_tmp)

        i += 1

    return scope, name, val, typ


# ------------------
# Checks if instruction has expected number of arguments
# ------------------
def check_number_args(instr, expected_no):
    if (len(list(instr)) != expected_no):
        print_exit("check_number_args -> wrong number of arguments in instruction", 32)
    
    i = 1
    for arg in instr:
        check_arg_seq(arg, i)
        i += 1


# ------------------
# Checks if arg[i] is well formed
# ------------------
def check_arg_seq(arg, cnt):
    arg_tag = "arg" + str(cnt)
    if (arg_tag != arg.tag):
        print_exit("check_arg_seq -> arg[i] not well-formed", 32)


# ------------------------------- INSTRUCTIONS -------------------------------


# -------------------
# ------ WRITE ------
# -------------------
def WRITE(instr):
    check_number_args(instr, 1)

    for arg in instr:
        check_arg(arg)
        val, typ = const_or_var(arg)
    
    if (typ == "float"):
        val = float.hex(val)
    
    print(val, end='')


# ------------------
# ------ READ ------
# ------------------
def READ(instr):
    global input_bool, file_handler
    check_number_args(instr, 2)
    
    if not (input_bool):
        input_content = input()
    else:
        input_content = file_handler.readline()

    i = 1
    for arg in instr:
        check_arg(arg)

        if (i == 1):
            check_var(arg)
            scope, name = scope_name_check_var(arg)

        if (i == 2):
            check_type(arg)
            typ = arg.text

        i += 1
    
    if (typ == "int"):
        try:
            input_content = re.sub(r"\n$", "", input_content)
            result = int(input_content)
            r_typ = "int"
        except Exception:
            result = "nil"
            r_typ = "nil"
    elif (typ == "string"):
        if input_content != "":
            input_content = re.sub(r"\n$", "", input_content)
            result = input_content
            r_typ = "string"
        else:
            result = "nil"
            r_typ = "nil"
    elif (typ == "bool"):
        input_content = re.sub(r"\n$", "", input_content)
        if (input_content == ""):
            result = "nil"
            r_typ = "nil"
        elif (input_content.upper() == "TRUE"):
            result = "true"
            r_typ = "bool"
        else:
            result = "false"
            r_typ = "bool"
    elif (typ == "float"):
        try:
            input_content = re.sub(r"\n$", "", input_content)
            result = float.fromhex(input_content)
            r_typ = "float"
        except Exception:
            result = "nil"
            r_typ = "nil"

    add_var_value(scope, name, result, r_typ)


# --------------------
# ------ DEFVAR ------
# --------------------
def DEFVAR(instr):
    check_number_args(instr, 1)

    for arg in instr:
        check_arg(arg)
        check_var(arg)
        scope, name = arg.text.split("@")
    
    check_var_redef(scope, name)
    add_var_value(scope, name, None, None)


# ------------------
# ------ MOVE ------
# ------------------
def MOVE(instr):
    check_number_args(instr, 2)
    scope, name, val, typ = iterate_var_symb(instr, 2)
    
    add_var_value(scope, name, val[0], typ[0])


# -------------------------
# ------ CREATEFRAME ------
# -------------------------
def CREATEFRAME():
    global TF, TF_type, TF_access

    TF.clear()
    TF_type.clear()
    TF_access = True


# -----------------------
# ------ PUSHFRAME ------
# -----------------------
def PUSHFRAME():
    global TF, TF_type, LF, LF_type, TF_access, pushed

    if not (TF_access):
        print_exit("PUSHFRAME -> TF frame not defined", 55)
    
    TF_copy = TF.copy()
    TF_type_copy = TF_type.copy()
    
    LF.append(TF_copy)
    LF_type.append(TF_type_copy)
    
    TF.clear()
    TF_type.clear()
    TF_access = False
    pushed.append(True)


# ----------------------
# ------ POPFRAME ------
# ----------------------
def POPFRAME():
    global TF, TF_type, LF, LF_type, TF_access, pushed

    if (not (LF[-1]) and (len(LF) == 1)) or (not pushed[-1]):
        print_exit("POPFRAME -> LF frame empty", 55)

    TF = LF.pop()
    TF_type = LF_type.pop()
    TF_access = True
    pushed.pop()


# -------------------
# ------- CALL ------
# -------------------
def CALL(instr):
    global curr_instr, call_stack, total_instr

    check_number_args(instr, 1)
    
    for arg in instr:
        check_arg(arg)
        check_label_jump(arg)
        jump = labels.get(arg.text) - 1

    call_stack.append(curr_instr)
    curr_instr = jump
    total_instr += 1


# ---------------------
# ------- RETURN ------
# ---------------------
def RETURN():
    global curr_instr, call_stack

    if not call_stack:
        print_exit("RETURN -> call stack is empty", 56)

    curr_instr = call_stack.pop()


# -------------------
# ------ LABEL ------
# -------------------
def LABEL(instr, index):
    global labels
    
    check_number_args(instr, 1)
    
    for arg in instr:
        check_arg(arg)
        check_label(arg)

        if (arg.text in labels):
            print_exit("LABEL -> label redefinition", 52)

        labels[arg.text] = index


# -------------------
# ------ JUMP -------
# -------------------
def JUMP(instr):
    global curr_instr, total_instr

    check_number_args(instr, 1)
    
    for arg in instr:
        check_arg(arg)
        check_arg_seq(arg, 1)
        check_label_jump(arg)
        jump = labels.get(arg.text) - 1

    curr_instr = jump
    total_instr += 1


# -----------------------------------
# ------ JUMPIFEQ / JUMPIFNEQ -------
# -----------------------------------
def JUMPIF(instr, eq, stack_instr):
    global curr_instr, total_instr, stack

    if not stack_instr:
        check_number_args(instr, 3)
        
        val = []
        typ = []

        i = 1
        for arg in instr:
            check_arg(arg)
            
            # <label>
            if (i == 1):
                check_label_jump(arg)
                jump = labels.get(arg.text) - 1

            # <symb>
            if (i == 2) or (i == 3):
                val_tmp, typ_tmp = const_or_var(arg)
                val.insert(i-2, val_tmp)
                typ.insert(i-2, typ_tmp)

            i += 1
    else:
        check_number_args(instr, 1)
        
        for arg in instr:
            check_arg(arg)
            check_label_jump(arg)
            jump = labels.get(arg.text) - 1

        if len(stack) < 2:
            print_exit("RELATION (stack) -> stack is empty", 56)
        val_type_1 = stack.pop()
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        val.append(val_type_1[0])
        typ.append(val_type_0[1])
        typ.append(val_type_1[1])
    
    if (typ[0] != typ[1]) and (typ[0] != "nil" and typ[1] != "nil"):
        print_exit("JUMPIF -> different operand types", 53)
    
    if (eq == "EQ"):
        if (str(val[0]) == str(val[1])):
            curr_instr = jump
            total_instr += 1
    if (eq == "NEQ"):
        if (str(val[0]) != str(val[1])):
            curr_instr = jump
            total_instr += 1


# ------------------
# ------ EXIT ------
# ------------------
def EXIT(instr):
    global stats_bool, stats_file, total_instr, max_vars

    check_number_args(instr, 1)

    for arg in instr:
        val, typ = const_or_var(arg)

    if (typ != "int"):
        print_exit("EXIT -> wrong operand type", 53)
    if not (0 <= int(val) <= 49):
        print_exit("EXIT -> number not in interval <0, 49>", 57)

    if (stats_bool):
        do_stats(stats_file, total_instr, max_vars)
    
    sys.exit(int(val))

# --------------------
# ------ CONCAT ------
# --------------------
def CONCAT(instr):
    global GF, LF, TF, GF_type, LF_type, TF_type

    check_number_args(instr, 3)
    scope, name, val, typ = iterate_var_symb(instr, 3)
    
    if (typ[0] != "string") or (typ[1] != "string"):
        print_exit("CONCAT -> only string operands can be contacenated", 53)

    result = val[0] + val[1]

    add_var_value(scope, name, result, "string")
    

# --------------------
# ------ STRLEN ------
# --------------------
def STRLEN(instr):
    global GF, LF, TF, GF_type, LF_type, TF_type

    check_number_args(instr, 2)
    scope, name, val, typ = iterate_var_symb(instr, 2)
    
    if (typ[0] != "string"):
        print_exit("STRLEN -> only string operands can be used", 53)

    add_var_value(scope, name, len(val[0]), "int")


# ---------------------
# ------ GETCHAR ------
# ---------------------
def GETCHAR(instr):
    check_number_args(instr, 3)
    scope, name, val, typ = iterate_var_symb(instr, 3)

    if (typ[0] != "string") or (typ[1] != "int"):
        print_exit("GETCHAR -> wrong operand types", 53)

    if (len(val[0]) - 1 < int(val[1])) or (int(val[1]) < 0):
        print_exit("GETCHAR -> index out range", 58)

    string = str(val[0])
    position = int(val[1])
    result = string[position]

    add_var_value(scope, name, result, "string")


# ---------------------
# ------ SETCHAR ------
# ---------------------
def SETCHAR(instr):
    global GF, LF, TF, GF_type, LF_type, TF_type, TF_access, pushed

    check_number_args(instr, 3)
    scope, name, val, typ = iterate_var_symb(instr, 3)
    
    if (typ[0] != "int") or (typ[1] != "string"):
        print_exit("SETCHAR -> operand type not valid", 53)

    index = int(val[0])
    string = str(val[1])

    if (scope == "GF"):
        if (GF_type[name] == None):
            print_exit("SETCHAR -> variable not defined", 56)
        if (GF_type[name] != "string"):
            print_exit("SETCHAR -> variable not string", 53)
        if (len(string) == 0) or (index < 0) or (index > len(GF[name]) - 1):
            print_exit("SETCHAR -> indexing out of range / empty string", 58)
        tmp = list(GF[name])
        tmp[index] = string[0]
        GF[name] = ""
        GF[name] = GF[name].join(tmp)
    if (scope == "LF"):
        if not (pushed[-1]):
            print_exit("SETCHAR -> LF frame not accessible", 55)
        if (LF_type[name] == None):
            print_exit("SETCHAR -> variable not defined", 56)
        if (LF_type[name] != "string"):
            print_exit("SETCHAR -> variable not string", 53)
        if (len(string) == 0) or (index < 0) or (index > len(LF[name]) - 1):
            print_exit("SETCHAR -> indexing out of range / empty string", 58)
        tmp = list(LF[name])
        tmp[index] = string[0]
        LF[-1][name] = ""
        LF[name] = LF[name].join(tmp)
    if (scope == "TF"):
        if not (TF_access):
            print_exit("SETCHAR -> TF frame not accessible", 55)
        if (TF_type[name] == None):
            print_exit("SETCHAR -> variable not defined", 56)
        if (TF_type[name] != "string"):
            print_exit("SETCHAR -> variable not string", 53)
        if (len(string) == 0) or (index < 0) or (index > len(TF[name]) - 1):
            print_exit("SETCHAR -> indexing out of range / empty string", 58)
        tmp = list(TF[name])
        tmp[index] = string[0]
        TF[-1][name] = ""
        TF[name] = TF[name].join(tmp)


# ------------------
# ------ TYPE ------
# ------------------
def TYPE(instr):
    check_number_args(instr, 2)

    i = 1
    for arg in instr:
        check_arg(arg)

        # <var>
        if (i == 1):
            scope, name = scope_name_check_var(arg)

        # <symb>
        if (i == 2):
            val, typ = const_or_var(arg, TYPE = True)

        i += 1

    if typ is None:
        typ = ""

    add_var_value(scope, name, typ, "string")


# -------------------------------------
# ------------- MATH ------------------
# - ADD  | SUB  | MUL  | IDIV  | DIV --
# - ADDS | SUBS | MULS | IDIVS | DIVS -
# -------------------------------------
def MATH(instr, operation, stack_instr):
    global stack
    if not stack_instr:
        check_number_args(instr, 3)
        scope, name, val, typ = iterate_var_symb(instr, 3)
    else:
        if len(stack) < 2:
            print_exit("MATH (stack) -> stack is empty", 56)
        val_type_1 = stack.pop()
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        val.append(val_type_1[0])
        typ.append(val_type_0[1])
        typ.append(val_type_1[1])

    if (typ[0] != "int" and typ[0] != "float") or (typ[1] != "int" and typ[1] != "float"):
        print_exit("MATH -> math instructions with int or float only", 53)
    
    if (typ[0] != typ[1]):
        print_exit("MATH -> instructions are not of the same type", 53)

    if (typ[0] == "int"):
        val[0] = int(val[0])
        val[1] = int(val[1])
    else:
        val[0] = float(val[0])
        val[1] = float(val[1])
    
    if (operation == "ADD" or operation == "ADDS"):
        result = val[0] + val[1]
    elif (operation == "SUB" or operation == "SUBS"):
        result = val[0] - val[1]
    elif (operation == "MUL" or operation == "MULS"):
        result = val[0] * val[1]
    elif (operation == "IDIV" or operation == "IDIVS"):
        if (typ[0] != "int"):
            print_exit("MATH -> IDIV operands not int", 53)
        if (val[1] == 0):
            print_exit("MATH -> zero division error", 57)
        result = val[0] // val[1]
    elif (operation == "DIV" or operation == "DIVS"):
        if (typ[0] != "float"):
            print_exit("MATH -> DIV operands not float", 53)
        if (val[1] == 0.0):
            print_exit("MATH -> zero division error", 57)
        result = val[0] / val[1]
    
    if not stack_instr:
        add_var_value(scope, name, result, typ[0])
    else:
        stack.append((result, typ[0]))


# -----------------------------------
# ----------- RELATION --------------
# --------- LT | GT  | EQ -----------
# -------- LTS | GTS | EQS ----------
# -----------------------------------
def RELATION(instr, operation, stack_instr):
    global stack

    if not stack_instr:
        check_number_args(instr, 3)
        scope, name, val, typ = iterate_var_symb(instr, 3)
    else:
        if len(stack) < 2:
            print_exit("RELATION (stack) -> stack is empty", 56)
        val_type_1 = stack.pop()
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        val.append(val_type_1[0])
        typ.append(val_type_0[1])
        typ.append(val_type_1[1])

    
    if (typ[0] != typ[1]) and (typ[0] != "nil") and (typ[1] != "nil"):
        print_exit("RELATION -> operands are not of the same type", 53)

    i = 0
    while (i < 2):
        if (typ[i] == "bool"):
            if (val[i] == "false"):
                val[i] = False
            else:
                val[i] = True
        elif (typ[i] == "string" or typ[i] == "nil"):
            val[i] = str(val[i])
        elif (typ[i] == "float"):
            val[i] = float(val[i])
        else:
            val[i] = int(val[i])
        i += 1

    if (operation == "LT" or operation == "LTS"):
        if (typ[0] == "nil") or (typ[1] == "nil"):
            print_exit("RELATION -> nil can be used in EQ only", 53)
        if (val[0] < val[1]):
            result = "true"
        else:
            result = "false" 
    elif (operation == "GT" or operation == "GTS"):
        if (typ[0] == "nil") or (typ[1] == "nil"):
            print_exit("RELATION -> nil can be used in EQ only", 53)
        if (val[0] > val[1]):
            result = "true"
        else:
            result = "false"
    else:
        if (val[0] == val[1]):
            result = "true"
        else:
            result = "false"

    if not stack_instr:
        add_var_value(scope, name, result, "bool")
    else:
        stack.append((result, "bool"))


# -------------------------------
# ----------- BOOLS -------------
# --------- AND | OR ------------
# -------- ANDS | ORS -----------
# -------------------------------
def BOOLS(instr, operation, stack_instr):
    global stack

    if not stack_instr:
        check_number_args(instr, 3)
        scope, name, val, typ = iterate_var_symb(instr, 3)
    else:
        if len(stack) < 2:
            print_exit("RELATION (stack) -> stack is empty", 56)
        val_type_1 = stack.pop()
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        val.append(val_type_1[0])
        typ.append(val_type_0[1])
        typ.append(val_type_1[1])
    
    if (typ[0] != "bool") or (typ[1] != "bool"):
        print_exit("BOOLS -> operands are not of the same type", 53)

    i = 0
    while (i < 2):
        if (val[i] == "false"):
            val[i] = False
        else:
            val[i] = True
        i += 1

    if (operation == "AND" or operation == "ANDS"):
        result = val[0] and val[1]
    else:
        result = val[0] or val[1]

    if (result == True):
        result = "true"
    else:
        result = "false"

    if not stack_instr:
        add_var_value(scope, name, result, "bool")
    else:
        stack.append((result, "bool"))


# ----------------------------
# ----------- NOT ------------
# ----------- NOTS -----------
# ----------------------------
def NOT(instr, stack_instr):
    global stack
    
    if not stack_instr:
        check_number_args(instr, 2)
        scope, name, val, typ = iterate_var_symb(instr, 2)
    else:
        if len(stack) < 1:
            print_exit("RELATION (stack) -> stack is empty", 56)
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        typ.append(val_type_0[1])
    
    if (typ[0] != "bool"):
        print_exit("NOT -> operand is not bool", 53)

    if (val[0] == "true"):
        result = "false"
    else:
        result = "true"

    if not stack_instr:
        add_var_value(scope, name, result, "bool")
    else:
        stack.append((result, "bool"))


# ----------------------
# ------ INT2CHAR ------
# ----------------------
def INT2CHAR(instr, stack_instr):
    global stack
    
    if not stack_instr:
        check_number_args(instr, 2)
        scope, name, val, typ = iterate_var_symb(instr, 2)
    else:
        if len(stack) < 1:
            print_exit("RELATION (stack) -> stack is empty", 56)
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        typ.append(val_type_0[1])

    if (typ[0] != "int"):
        print_exit("INT2CHAR -> argument value not valid", 53)
    
    try:
        result = chr(int(val[0]))
    except Exception:
        print_exit("INT2CHAR -> argument value not valid", 58)
    
    if not stack_instr:
        add_var_value(scope, name, result, "string")
    else:
        stack.append((result, "string"))


# ----------------------
# ------ STRI2INT ------
# ----------------------
def STRI2INT(instr, stack_instr):
    global stack

    if not stack_instr:
        check_number_args(instr, 3)
        scope, name, val, typ = iterate_var_symb(instr, 3)
    else:
        if len(stack) < 2:
            print_exit("RELATION (stack) -> stack is empty", 56)
        val_type_1 = stack.pop()
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        val.append(val_type_1[0])
        typ.append(val_type_0[1])
        typ.append(val_type_1[1])

    if (typ[0] != "string") or (typ[1] != "int"):
        print_exit("STR2INT -> wrong operand types", 53)
    
    string = list(val[0])
    position = int(val[1])

    if (len(string) == 0) or (position < 0):
        print_exit("STRI2INT -> indexing out of range / empty string", 58)

    try:
        result = ord(string[position])
    except Exception:
        print_exit("STR2INT -> argument value not valid", 58)

    if not stack_instr:
        add_var_value(scope, name, result, "int")
    else:
        stack.append((result, "int"))

# -----------------------
# ------ INT2FLOAT ------
# -----------------------
def INT2FLOAT(instr, stack_instr):
    global stack
    
    if not stack_instr:
        check_number_args(instr, 2)
        scope, name, val, typ = iterate_var_symb(instr, 2)
    else:
        if len(stack) < 1:
            print_exit("RELATION (stack) -> stack is empty", 56)
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        typ.append(val_type_0[1])

    if (typ[0] != "int"):
        print_exit("INT2FLOAT -> argument value not valid", 53)
    
    try:
        result = float(val[0])
    except Exception:
        print_exit("INT2FLOAT -> argument value not valid", 53)
    
    if not stack_instr:
        add_var_value(scope, name, result, "float")
    else:
        stack.append((result, "float"))


# -----------------------
# ------ FLOAT2INT ------
# -----------------------
def FLOAT2INT(instr, stack_instr):
    global stack
    
    if not stack_instr:
        check_number_args(instr, 2)
        scope, name, val, typ = iterate_var_symb(instr, 2)
    else:
        if len(stack) < 1:
            print_exit("RELATION (stack) -> stack is empty", 56)
        val_type_0 = stack.pop()
        val = []
        typ = []
        val.append(val_type_0[0])
        typ.append(val_type_0[1])

    if (typ[0] != "float"):
        print_exit("FLOAT2INT -> argument value not valid", 53)
    
    try:
        result = int(val[0])
    except Exception:
        print_exit("FLOAT2INT -> argument value not valid", 53)
    
    if not stack_instr:
        add_var_value(scope, name, result, "int")
    else:
        stack.append((result, "int"))


# --------------------
# ------ DPRINT ------
# --------------------
def DPRINT(instr):
    global stats_bool, stats_file, total_instr, max_vars
    
    check_number_args(instr, 1)

    for arg in instr:
        val, typ = const_or_var(arg)

    if (stats_bool):
        do_stats(stats_file, total_instr, max_vars)
    
    sys.stderr.write(val)

# --------------------
# ------ BREAK -------
# --------------------
def BREAK():
    global GF, LF, TF, total_instr, curr_instr

    print("------ BREAK -------", file=sys.stderr)
    print("Global frame: ", GF, file=sys.stderr)
    print("Local frame: ", LF, file=sys.stderr)
    print("Temporary frame: ", TF, file=sys.stderr)
    print("Total number of executed functions: ", total_instr, file=sys.stderr)
    print("Current instruction: ", curr_instr, file=sys.stderr)
    print("--------------------", file=sys.stderr)


# -------------------
# ------ PUSHS ------
# -------------------
def PUSHS(instr):
    global stack

    check_number_args(instr, 1)

    for arg in instr:
        check_arg_seq(arg, 1)
        check_arg(arg)
        val, typ = const_or_var(arg)

    stack.append((val, typ))


# -------------------
# ------ POPS ------
# -------------------
def POPS(instr):
    global stack

    check_number_args(instr, 1)
    scope, name, _, _ = iterate_var_symb(instr, 1)

    if not stack:
        print_exit("POPS -> stack is empty", 56)

    val, typ = stack.pop()
    add_var_value(scope, name, val, typ)


# --------------------
# ------ CLEARS ------
# --------------------
def CLEARS():
    global stack
    stack.clear()


# -------------------------
# ------ MAIN SWITCH ------
# -------------------------
def interpret(instr):
    opcode = instr.attrib["opcode"].upper()

    if opcode == "MOVE":
        MOVE(instr)
    elif opcode == "CREATEFRAME":
        CREATEFRAME()
    elif opcode == "PUSHFRAME":
        PUSHFRAME()
    elif opcode == "POPFRAME":
        POPFRAME()
    elif opcode == "DEFVAR":
        DEFVAR(instr)
    elif opcode == "CALL":
        CALL(instr)
    elif opcode == "RETURN":
        RETURN()
    elif opcode == "PUSHS":
        PUSHS(instr)
    elif opcode == "POPS":
        POPS(instr)
    elif opcode == "CLEARS":
        CLEARS()
    elif opcode == "ADD":
        MATH(instr, "ADD", False)
    elif opcode == "SUB":
        MATH(instr, "SUB", False)
    elif opcode == "MUL":
        MATH(instr, "MUL", False)
    elif opcode == "IDIV":
        MATH(instr, "IDIV", False)
    elif opcode == "DIV":
        MATH(instr, "DIV", False)
    elif opcode == "ADDS":
        MATH(instr, "ADDS", True)
    elif opcode == "SUBS":
        MATH(instr, "SUBS", True)
    elif opcode == "MULS":
        MATH(instr, "MULS", True)
    elif opcode == "IDIVS":
        MATH(instr, "IDIVS", True)
    elif opcode == "DIVS":
        MATH(instr, "DIVS", True)
    elif opcode == "LT":
        RELATION(instr, "LT", False)
    elif opcode == "GT":
        RELATION(instr, "GT", False)
    elif opcode == "EQ":
        RELATION(instr, "EQ", False)
    elif opcode == "LTS":
        RELATION(instr, "LTS", True)
    elif opcode == "GTS":
        RELATION(instr, "GTS", True)
    elif opcode == "EQS":
        RELATION(instr, "EQS", True)
    elif opcode == "AND":
        BOOLS(instr, "AND", False)
    elif opcode == "OR":
        BOOLS(instr, "OR", False)
    elif opcode == "ANDS":
        BOOLS(instr, "ANDS", True)
    elif opcode == "ORS":
        BOOLS(instr, "ORS", True)
    elif opcode == "NOT":
        NOT(instr, False)
    elif opcode == "NOTS":
        NOT(instr, True)
    elif opcode == "INT2CHAR":
        INT2CHAR(instr, False)
    elif opcode == "INT2CHARS":
        INT2CHAR(instr, True)
    elif opcode == "STRI2INT":
        STRI2INT(instr, False)
    elif opcode == "STRI2INTS":
        STRI2INT(instr, True)
    elif opcode == "INT2FLOAT":
        INT2FLOAT(instr, False)
    elif opcode == "INT2FLOATS":
        INT2FLOAT(instr, True)
    elif opcode == "FLOAT2INT":
        FLOAT2INT(instr, False)
    elif opcode == "FLOAT2INTS":
        FLOAT2INT(instr, True)
    elif opcode == "WRITE":
        WRITE(instr)
    elif opcode == "READ":
        READ(instr)
    elif opcode == "CONCAT":
        CONCAT(instr)
    elif opcode == "STRLEN":
        STRLEN(instr)
    elif opcode == "GETCHAR":
        GETCHAR(instr)
    elif opcode == "SETCHAR":
        SETCHAR(instr)
    elif opcode == "TYPE":
        TYPE(instr)
    elif opcode == "LABEL":
        return
    elif opcode == "JUMP":
        JUMP(instr)
    elif opcode == "JUMPIFEQ":
        JUMPIF(instr, "EQ", False)
    elif opcode == "JUMPIFNEQ":
        JUMPIF(instr, "NEQ", False)
    elif opcode == "JUMPIFEQS":
        JUMPIF(instr, "EQ", True)
    elif opcode == "JUMPIFNEQS":
        JUMPIF(instr, "NEQ", True)
    elif opcode == "EXIT":
        EXIT(instr)
    elif opcode == "DPRINT":
        DPRINT(instr)
    elif opcode == "BREAK":
        BREAK()
    else:
        print_exit("Unknown opcode", 32)


# ----------------------
# -------- MAIN --------
# ----------------------

# ----- Variables ------
curr_instr = 0

# Switch variables
source_bool = False
input_bool = False
source_file = None
input_file = None

# Variables in scopes
GF = {}
TF = {}
LF = [{}]

GF_type = {}
TF_type = {}
LF_type = [{}]

pushed = [False]
TF_access = False

# Others
stack = []
call_stack = []
instructions = []
labels = {}

# Stats
stats_file = ""
stats_bool = False
total_instr = 0
max_vars = 0

# ----- JUST DO IT ------

parse_args()
source_content = read_source(source_bool, source_file)

if input_bool:
    try:
        file_handler = open(input_file, "r")
    except Exception:
        print_exit("Could not open input file", 11)

# Save XML representation and check well-formedness
try:
    xml = et.fromstring(source_content)
except et.ParseError as err:
    print_exit("MAIN -> XML not well-formed", 31)

# Root XML attribute check
if "language" not in xml.attrib:
    print_exit("MAIN -> XML language attribute missing", 32)
if (xml.attrib["language"].upper() != "IPPCODE20"):
    print_exit("Wrong header", 32)
if (len(xml.attrib) < 1) or (len(xml.attrib) > 3):
    print_exit("MAIN -> XML attribute length error", 32)
for attrib in xml.attrib:
    if not re.match(r"name|description|language", attrib):
        print_exit("MAIN -> XML attribute name error", 32)

# Main XML tag
if (xml.tag != "program"):
    print_exit("Wrong XML format", 32)

# Sort XML instructions by order and attributes by tag
# Check if order of instruction is used only once
try:
    xml[:] = sorted(xml, key=lambda child: (child.tag, int(child.get("order"))))
    order = []
    for instr in xml:
        if instr.attrib["order"] in order:
            print_exit("MAIN -> XML intruction order used several times", 32)
        else:
            order += instr.attrib["order"]
        check_instr(instr)
        instr[:] = sorted(instr, key=lambda child: (child.tag))
except Exception:
    print_exit("MAIN -> XML intruction error", 32)

# Save labels to dictionary in form {name : index}
cnt = 1
for label in xml:
    if "LABEL" in list(label.attrib.values())[1].upper():
        label_index = cnt
        LABEL(label, label_index)
    cnt += 1

# Save all instructions to array instructions
for instr in xml:
    instructions.append(instr)

if (len(instructions) == 0):
    if (stats_bool):
        do_stats(stats_file, total_instr, max_vars)
    exit(0)

# Loop through all instructions based on curr_instr counter
# curr_instr is changed dynamically in JUMP/CALL instructions
try:
    max_vars = 0
    while True:
        total_instr += 1
        interpret(instructions[curr_instr])
        curr_instr += 1
        max_vars = count_vars(max_vars)
        if (curr_instr >= len(instructions)):
            break
except Exception:
    print_exit("MAIN -> wrong XML format", 32)

if (stats_bool):
    do_stats(stats_file, total_instr, max_vars)
