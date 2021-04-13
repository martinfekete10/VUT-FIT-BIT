# Name:   ISA projekt - Testovací skript
# Author: Martin Fekete <xfeket00@stud.fit.vutbr.cz>
# Date:   29.10.2020

import os
import signal
import subprocess
import random

# Colors in terminal output
RED = '\33[31m'
GREEN = '\33[32m'
END = '\33[0m'

total_test = 0
passed_test = 0

port = random.randint(1024, 65535)

# Tested domains
valid_domains = ["bazos.sk", "seznam.cz", "apple.com", "lenovo.com", "youtube.com", "samsung.com", "vutbr.cz", "agoogle.com"]
blacklist_domains = ["google.com", "facebook.com", "drive.google.com", "docs.google.com", "business.facebook.com"]
not_imp = ["NS", "MD", "MF", "CNAME", "SOA", "MB", "MG", "MR", "NULL", "WKS", "PTR", "HINFO", "MINFO", "MX", "TXT"]


############################################
#                  IPv4
############################################

print("")
print("--------------------------------")
print('IPv4')
print("--------------------------------")

# ------------------------------------
# Run server
cmd = "./dns -s 1.1.1.1 -f ./tests/test_filter.txt -p " + str(port)
server = subprocess.Popen(cmd, shell=True, preexec_fn=os.setsid)

# ------------------------------------
# Test valid domains
print("")
print('IPv4 valid domains')
print("--------------------------------")

for domain in valid_domains:
    cmd = "dig @127.0.0.1 -p " + str(port) + " " + str(domain) + " | grep '^;; flags:' > ./tests/out1.txt"
    os.system(cmd)

    cmd = "dig @1.1.1.1 " + str(domain) + " | grep '^;; flags:' > ./tests/out2.txt"
    os.system(cmd)

    print(domain, end = " : ")

    stream = os.popen("diff ./tests/out1.txt ./tests/out2.txt")
    if  (stream.read() == ""):
        print(GREEN + 'PASSED' + END)
        passed_test += 1
    else:
        print(RED + 'FAILED' + END)
    
    total_test += 1

os.system("rm ./tests/out1.txt")
os.system("rm ./tests/out2.txt")


# ------------------------------------
# Test REFUSED domains
print("")
print('IPv4 REFUSED domains')
print("--------------------------------")

for domain in blacklist_domains:
    cmd = "dig @127.0.0.1 -p " + str(port) + " " + str(domain) + " | awk -v FS=\"(status: |,)\" '{print $3}' | grep -m 1 . > ./tests/out1.txt"
    os.system(cmd)

    print(domain, end = " : ")

    stream = os.popen("diff ./tests/out1.txt ./tests/blacklist_out.txt")
    if  (stream.read() == ""):
        print(GREEN + 'PASSED' + END)
        passed_test += 1
    else:
        print(RED + 'FAILED' + END)

    total_test += 1
    
os.system("rm ./tests/out1.txt")

# ------------------------------------
# Test NOTIMP domains
print("")
print('IPv4 NOTIMP domains')
print("--------------------------------")

for notimp in not_imp:
    cmd = "dig @127.0.0.1 -p " + str(port) + " bazos.sk -t " + notimp + " | awk -v FS=\"(status: |,)\" \'{print $3}\' | grep -m 1 . > ./tests/out1.txt"
    os.system(cmd)

    print(notimp, end = " : ")

    stream = os.popen("diff ./tests/out1.txt ./tests/notimp_out.txt")
    if  (stream.read() == ""):
        print(GREEN + 'PASSED' + END)
        passed_test += 1
    else:
        print(RED + 'FAILED' + END)

    total_test += 1
    
os.system("rm ./tests/out1.txt")

# ------------------------------------
# Kill server
os.killpg(os.getpgid(server.pid), signal.SIGTERM)

############################################
#                  IPv6
############################################

print("")
print("--------------------------------")
print('IPv6')
print("--------------------------------")


# ------------------------------------
# Run server
cmd = "./dns -s 2001:4860:4860::8888 -f ./tests/test_filter.txt -p " + str(port)
server = subprocess.Popen(cmd, shell=True, preexec_fn=os.setsid)

# ------------------------------------
# Test valid domains
print("")
print('IPv6 valid domains')
print("--------------------------------")

for domain in valid_domains:
    cmd = "dig @::1 -6 -p " + str(port) + " " + str(domain) + " | grep '^;; flags:' > ./tests/out1.txt"
    os.system(cmd)

    cmd = "dig @2001:4860:4860::8888 -6 " + str(domain) + " | grep '^;; flags:' > ./tests/out2.txt"
    os.system(cmd)

    print(domain, end = " : ")

    stream = os.popen("diff ./tests/out1.txt ./tests/out2.txt")
    if  (stream.read() == ""):
        print(GREEN + 'PASSED' + END)
        passed_test += 1
    else:
        print(RED + 'FAILED' + END)
    
    total_test += 1

os.system("rm ./tests/out1.txt")
os.system("rm ./tests/out2.txt")


# ------------------------------------
# Test REFUSED domains
print("")
print('IPv6 REFUSED domains')
print("--------------------------------")

for domain in blacklist_domains:
    cmd = "dig @::1 -6 -p " + str(port) + " " + str(domain) + " | awk -v FS=\"(status: |,)\" '{print $3}' | grep -m 1 . > ./tests/out1.txt"
    os.system(cmd)

    print(domain, end = " : ")

    stream = os.popen("diff ./tests/out1.txt ./tests/blacklist_out.txt")
    if  (stream.read() == ""):
        print(GREEN + 'PASSED' + END)
        passed_test += 1
    else:
        print(RED + 'FAILED' + END)

    total_test += 1
    
os.system("rm ./tests/out1.txt")

# ------------------------------------
# Test NOTIMP domains
print("")
print('IPv6 NOTIMP domains')
print("--------------------------------")

for notimp in not_imp:
    cmd = "dig @::1 -6 -p " + str(port) + " bazos.sk -t " + notimp + " | awk -v FS=\"(status: |,)\" \'{print $3}\' | grep -m 1 . > ./tests/out1.txt"
    os.system(cmd)

    print(notimp, end = " : ")

    stream = os.popen("diff ./tests/out1.txt ./tests/notimp_out.txt")
    if  (stream.read() == ""):
        print(GREEN + 'PASSED' + END)
        passed_test += 1
    else:
        print(RED + 'FAILED' + END)

    total_test += 1
    
os.system("rm ./tests/out1.txt")

# ------------------------------------
# Kill server
os.killpg(os.getpgid(server.pid), signal.SIGTERM)

############################################
#             IPv4-mapped-IPv6
############################################

print("")
print("--------------------------------")
print('IPv4-mapped-IPv6')
print("--------------------------------")


# ------------------------------------
# Run server
cmd = "./dns -s ::ffff:8.8.8.8 -f ./tests/test_filter.txt -p " + str(port)
server = subprocess.Popen(cmd, shell=True, preexec_fn=os.setsid)

for domain in valid_domains:
    cmd = "dig @::1 -p " + str(port) + " " + str(domain) + " | grep '^;; flags:' > ./tests/out1.txt"
    os.system(cmd)

    cmd = "dig @8.8.8.8 " + str(domain) + " | grep '^;; flags:' > ./tests/out2.txt"
    os.system(cmd)

    print(domain, end = " : ")

    stream = os.popen("diff ./tests/out1.txt ./tests/out2.txt")
    if  (stream.read() == ""):
        print(GREEN + 'PASSED' + END)
        passed_test += 1
    else:
        print(RED + 'FAILED' + END)
    
    total_test += 1

os.system("rm ./tests/out1.txt")
os.system("rm ./tests/out2.txt")

# ------------------------------------
# Kill server
os.killpg(os.getpgid(server.pid), signal.SIGTERM)

# ------------------------------------
# Print stats
print("")
print("--------------------------------")
print("Total: " + str(passed_test) + "/" + str(total_test) + "\n")

