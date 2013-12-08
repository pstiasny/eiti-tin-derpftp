Przeznaczenie
=============

W ramach projektu powstanie:

* protokół,
* serwer,
* biblioteka klienta,
* referencyjny klient

służące zdalnemu dostępowi do plików, w tym pobierania i wysyłania do zdalnego
drzewa katalogów plików i fragmentów plików poprzez interfejs pogramistyczny
zbliżony do standardu POSIX odnośnie operacji na plikach.


Protokół
========

W protokle wykorzystywany jest zbiór typów poleceń wraz z maksymalnie
dwoma 32-bitowymi argumentami.  Typy są identyfikowane 8-bitowymi liczbami.
Odpowiedź na polecenie zawiera 32-bitową liczbę zawierającą kod sygnalizujący
powodzenie lub typ błędu, oraz opcjonalną 32-bitową wartość `val`.
Dopuszczalne typy poleceń to:

    FSMSG_TYPE     | type | arg1     | arg2   | val
    ---------------+------+----------+--------+-----
    FSMSG_OPEN     | 0    | flags    |        | fd
    FSMSG_WRITE    | 1    | len      |        |
    FSMSG_READ     | 2    | len      |        | len
    FSMSG_LSEEK    | 3    | offset   | whence |
    FSMSG_CLOSE    | 4    |          |        |
    FSMSG_STAT     | 5    |          |        |


Komunikacja między klientem i serwerem odbywa się w sposób następujący:

1. Klient otwiera połączenie TCP z serwerem.
2. Klient wysyła komunikat polecenia (patrz Struktury Danych).
3. Jeśli typ polecenia to FSMSG_WRITE, to klient wysyła strumień oktetów
   o długości określonej przez argument `len` polecenia.
4. Klient odbiera komunikat odpowiedzi (patrz Struktury Danych).
5. Jeśli typ polecenia to FSMSG_READ, to klient odbiera strumień oktetów
   o długości podanej przez serwer w polu `len` odpowiedzi.
6. Idź do 2 lub klient zamyka połączenie.


Struktury Danych
----------------

Wszystkie liczbowe typy danych są przesyłane w kolejści bajtów
cienko-końcówkowej (little-endian).  Typ polecenia jest identyfikowany
przez pierwszy bajt `type` struktury polecenia.

Struktura używana przez FSMSG_WRITE, FSMSG_READ, FSMSG_LSEEK, FSMSG_CLOSE
oraz FSMSG_STAT:

        +0    +8    +16   +24   +32   +40   +48   +56   +64
         +-----+-----------------+-----------------------+
    0    |type |                 |fd                     |
         +-----+-----------------+-----------------------+
    64   |arg1                   |arg2                   |
         +-----------------------+-----------------------+

Struktura używana przez FSMSG_OPEN:

        +0    +8    +16   +24   +32   +40   +48   +56   +64
         +-----+-----------------------------------------+
    0    |type |                                         |
         +-----+-----------------+-----------------------+
    64   |flags                  |                       |
         +-----------------------+-----------------------+
    128  |filename                                       |
         |                                               |
         |                                               |
    ...  |                                               |
         |                                               |
         |                                               |
    2112 |                                               |
         +-----------------------------------------------+

Podstawowa struktura odpowiedzi serwera:

        +0                      +32                     +64
         +-----------------------+-----------------------+
    0    |status                 |val                    |
         +-----------------------+-----------------------+


Struktura projektu
==================

Kod projektu powstaje w repozytorium dostępnym git pod adresem:

<https://github.com/pstiasny/eiti-tin-derpftp>.

Struktura modułów wygląda obecnie następująco:

    Makefile
    src/
        api.h         - API biblioteki klienta: funkcje, definicje kodów błędów
        types.h       - definicje struktur danych
        client.c      - implementacja biblioteki klienta
        testclient.c  - implementacja klienta testowego
        server.c      - implementacja serwera
    lib/
        libderpftp.a  - statycznie dołączana biblioteka klienta
    bin/
        derpftpd      - program serwera
        testclient    - interaktywny klient
    tests/            - skrypty i dane środowiska testowego

Kompilacja całego projektu odbywa się za pomocą standardowego narzędzia `make`.


Interfejs
---------

Zgodnie z treścią zadania, biblioteka udostępnia następujące funkcje,
zdefiniowane w `api.h`:

    int fs_open_server(const char *server_addr);
    int fs_close_server(int server_handle);
    int fs_open(int server_handle, const char *name, int flags);
    int fs_write(int server_handle, int fd, void *buf, size_t len);
    int fs_read(int server_handle, int fd, void *buf, size_t len);
    int fs_lseek(int server_handle, int fd, long offset, int whence);
    int fs_close(int server_handle, int fd);
    int fs_fstat(int server_handle, int fd, struct stat *buf);


Współbieżność
-------------

Serwer dla każdego przychodzącego połączenia otwierta podproces za pomocą
funkcji `fork`.  Każdy podproces przechowuje stan sesji dla danego połączenia,
w szczególności deskryptory otwartych plików -- używane w komunikatach
deskryptory odpowiadają deskryptorom plików przypisanych przez system
operacyjny do procesu.  Sesja wygasa po rozłączeniu klienta lub po upłenięciu
ustalonego czasu.  Serwer ogranicza liczbę maksymalnie uruchomionych sesji
do ustalonej liczby.


Procedura testowania
--------------------

Środowisko testowe składa się z:

* pobierającej ze standardowego wejścia komendy implementacji klienta,
* programu netcat,
* zestawu referencyjnych zapisów strumieni TCP w postaci szesnastkowej,
* zestawu skryptów w języku bash używających powyższych do weryfikacji
  poprawności.

Przykład pracy z interaktywnym klientem testowym - polecenie otwarcia pliku
PLIK:

klient:

    $ bin/testclient localhost
    fs_open_server returned 0
    open PLIK
    fs_open returned 1
    fs_close_server returned 0

emulacja serwera:

    $ nc -l -x -p 1337 -x < tests/1-in-nc.bin > /dev/null
    Sent 8 bytes to the socket
    00000000  00 00 00 00  01 00 00 00                            ........        
    Received 272 bytes from the socket
    00000000  00 00 00 00  00 00 00 00  10 00 00 00  00 00 00 00  ................
    00000010  50 4C 49 4B  00 00 00 00  00 00 00 00  00 00 00 00  PLIK............
    00000020  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    00000030  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    00000040  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    00000050  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    00000060  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    00000070  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    00000080  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    00000090  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    000000A0  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    000000B0  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    000000C0  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    000000D0  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    000000E0  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    000000F0  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................
    00000100  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  ................


