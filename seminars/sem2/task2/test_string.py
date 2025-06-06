#!/usr/bin/python2

import os
import subprocess
import sys
import re
import sys
from subprocess import CalledProcessError, Popen, PIPE

#-------helpers---------------

def starts_uint( s ):
    matches = re.findall('^\d+', s)
    if matches:
        return (int(matches[0]), len(matches[0]))
    else:
        return (0, 0)

def starts_int( s ):
    matches = re.findall('^-?\d+', s)
    if matches:
        return (int(matches[0]), len(matches[0]))
    else:
        return (0, 0)

def unsigned_reinterpret(x):
    if x < 0:
        return x + 2**64
    else:
        return x

def first_or_empty( s ):
    sp = s.split()
    if sp == [] :
        return ''
    else:
        return sp[0]

#-----------------------------

def compile( fname, text ):
    f = open( fname + '.asm', 'w')
    f.write( text )
    f.close()

    subprocess.call( ['nasm', '--version'] )

    if subprocess.call( ['nasm', '-f', 'elf64', fname + '.asm', '-o', fname+'.o'] ) == 0 and subprocess.call( ['ld', '-o' , fname, fname+'.o'] ) == 0:
             print ' ', fname, ': compiled'
             return True
    else:
        print ' ', fname, ': failed to compile'
        return False


def launch( fname, seed = '' ):
    output = ''
    try:
        p = Popen(['./'+fname], shell=None, stdin=PIPE, stdout=PIPE)
        (output, err) = p.communicate(input=seed)
        return (output, p.returncode)
    except CalledProcessError as exc:
        return (exc.output, exc.returncode)
    else:
        return (output, 0)



def test_asm( text, name = 'dummy',  seed = '' ):
    if compile( name, text ):
        r = launch( name, seed )
        #os.remove( name )
        #os.remove( name + '.o' )
        #os.remove( name + '.asm' )
        return r
    return None

class Test:
    name = ''
    string = lambda x : x
    checker = lambda input, output, code : False

    def __init__(self, name, stringctor, checker):
        self.checker = checker
        self.string = stringctor
        self.name = name
    def perform(self, arg):
        res = test_asm( self.string(arg), self.name, arg)
        if res is None:
            return False
        (output, code) = res
        print '"', arg,'" ->',  res
        return self.checker( arg, output, code )

before_call="""
mov rdi, -1
mov rsi, -1
mov rax, -1
mov rcx, -1
mov rdx, -1
mov r8, -1
mov r9, -1
mov r10, -1
mov r11, -1
push rbx
push rbp
push r12
push r13
push r14
push r15
"""
after_call="""
cmp r15, [rsp]
jne .convention_error
pop r15
cmp r14, [rsp]
jne .convention_error
pop r14
cmp r13, [rsp]
jne .convention_error
pop r13
cmp r12, [rsp]
jne .convention_error
pop r12
cmp rbp, [rsp]
jne .convention_error
pop rbp
cmp rbx, [rsp]
jne .convention_error
pop rbx

jmp continue

.convention_error:
    mov rax, 1
    mov rdi, 2
    mov rsi, err_calling_convention
    mov rdx,  err_calling_convention.end - err_calling_convention
    syscall
    mov rax, 60
    mov rdi, -41
    syscall
section .data
err_calling_convention: db "You did not respect the calling convention! Check that you handled caller-saved and callee-saved registers correctly", 10
.end:
section .text
continue:
"""
tests=[ Test('string_length',
             lambda v : """section .data
        str: db '""" + v + """', 0
        section .text
        %include "lib.inc"
        global _start
        _start:
        """ + before_call + """
        mov rdi, str
        call string_length
        """ + after_call + """
        mov rdi, rax
        mov rax, 60
        syscall""",
        lambda i, o, r: r == len(i)
         ),

        Test('print_string',
             lambda v : """section .data
        str: db '""" + v + """', 0
        section .text
        %include "lib.inc"
        global _start
        _start:
        """ + before_call + """
        mov rdi, str
        call print_string
        """ + after_call + """

        mov rax, 60
        xor rdi, rdi
        syscall""",
        lambda i,o,r: i == o)
        ]


inputs= {'string_length'
         : [ 'asdkbasdka', 'qwe qweqe qe', ''],
         'print_string'
         : ['ashdb asdhabs dahb', ' ', '']
}

if __name__ == "__main__":
    found_error = False
    for t in tests:
        for arg in inputs[t.name]:
            if not found_error:
                try:
                    print '          testing', t.name,'on "'+ arg +'"'
                    res = t.perform(arg)
                    if res:
                        print '  [  ok  ]'
                    else:
                        print '* [ fail]'
                        found_error = True
                except:
                    print '* [ fail] with exception' , sys.exc_info()[0]
                    raise
                    found_error = True
    if found_error:
        sys.exit('Not all tests have been passed')
    else:
        print "Good work, all tests are passed"
