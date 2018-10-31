#!/usr/bin/python2.7
import sys
import os
import time
from subprocess import Popen, PIPE


def generate(if_num):
    path = "s1/"
    os.system("rm -rf " + path)
    os.system("mkdir -p " + path)
    no = 0
    while no < 10:
        target = path + "test" + str(if_num) + "_" + str(no)
        re = os.system("./dual-generator " + target + " " + str(if_num * 5) + " " + str(if_num * 50) + " " + str(if_num))
        time.sleep(1)
        while re != 0:
            re = os.system("./dual-generator " + target + " " + str(if_num * 5) + " " + str(if_num * 50) + " " + str(if_num))
            time.sleep(1)
        no += 1 
    

def main():
    if_num = 1 
    while if_num <= 10:
        generate(if_num)
        if_num += 1 

if __name__ == "__main__":
    main()
