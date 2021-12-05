CFLAGS = -Wall -Wextra
OPENSSL = -lcrypto

filevault:
	gcc ${CFLAGS} filevault.c -o filevault ${OPENSSL}

clean:
	rm -f filevault ${HOME}/.filevault_key