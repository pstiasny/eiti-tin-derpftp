
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

        +0    +8    +16   +32
         +-----+-----------+
    0    |type |           |
         +-----+-----------+
    32   |fd               |
         +-----------------+
    64   |arg1             |
         +-----------------+
    96   |arg2             |
         +-----------------+

Struktura używana przez FSMSG_OPEN:

        +0    +8    +16   +32
         +-----+-----------+
    0    |type |           |
         +-----+-----------+
    32   |fd               |
         +-----------------+
    64   |arg1             |
         +-----------------+
    96   |arg2             |
         +-----------------+
    128  |filename         |
         |                 |
    160  |                 |
         |                 |
    192  |                 |
         |                 |
    224  |                 |
         |                 |
    256  |                 |
         |                 |
    288  |                 |
         |                 |
    320  |                 |
         |                 |
    352  |                 |
         +-----------------+

Podstawowa struktura odpowiedzi serwera:

        +0    +8    +16   +32
         +-----------------+
    0    |status           |
         +-----------------+
    32   |val              |
         +-----------------+

