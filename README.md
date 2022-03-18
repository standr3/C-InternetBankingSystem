# C-InternetBankingSystem


# [Server side]

Structura Entry : Pe langa campurile precizate in enunt am adaugat doua campuri int -> locked (reprezinta starea contului - blocat sau nu), failed_logins (numarul de incercari de logare -> daca este mai mare sau egal cu 3 atunci contul va fi blocat)
Structura Session : Reprezinta o sesiune creata de logarea unui utilizator. Caracterizata printr-un Entry care este accesat de client si PID-ul clientului.

Functia showEntry : Functie pentru debuggare; afiseaza un Entry din cele gasite in fisierul de input al serverului.
Functia error : Afiseaza eroare in terminal si apeleaza functia exit().
Functia checkServerPort : Verifica daca portul introdus serverul este valid (contine cifre), altfel apeleaza functia error()
Functia checkServerData : Verifica daca fisierul de input pt userData poate fi deschis.
Functia getUserData : Citeste toate datele fiecarui utilizator din fisierul de input.
Functia getEntry : Returneaza adresa la care un entry se afla in memorie.
Functia getUserByCardNumber : Returneaza 1 daca un utilizator este gasit in baza de date, primind ca input numarul cardului; returneaza 0 daca utilizatorul nu e gasit.
Functia getSessionEntry : Se returneaza un entry pe baza PID-ului clientului; se cauta in sessiunile active.
Functia activeSession: Verifica daca o sessiune e activa sau nu.
Functia activePID : Verifica daca exista o sesiune activa care sa contina PID-ul introdus.
Functia addSession : Adauga o noua sesiune cu un entry si un PID introdus ca argumente.
Functia rmvSession : Indeparteaza o sesiune din lista sesiunile active.

Precizari suplimentare: 
- fiecare comanda este salvata intr-un char[] si daca comanda este valida atunci se executa acea comanda iar variabila valid_command devine 1; altfel operatia incercata va esua.
- functia unlock este implementata tot pe baza socketului TCP
- daca se doreste acceptarea doar unui fisier de input cu numele users_data_file atunci va trebui decomentat codul din functia checkServerData
- nu este implementata functionalitatea "quit" pentru server

# [Client side]

Functiile lenHelper si printLen : Pentru aflarea numarului de cifre ale PID-ului.

Precizari suplimentare: 
- este implementata functionalitatea "quit" pentru client
- este recomandat ca, pentru a nu aparea probleme la scrierea in fisier, clientul sa incheie procesul cu ajutorul quit
