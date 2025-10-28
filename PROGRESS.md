# DELETE THIS FILE BEFORE MERGE

* no more runtime errors, but the instance list on the gui shows instances of the same Mode instead
  of possible connections...
  - fixed

* nincs hang
  - véletlenül eddit a transmitterekhez pusholtam a sample-eket, amiket a transmitterek küldenek...
  - átmozgattam a reciever loop-ba, és így crashel a plugin
  - CircularBuffer a Core-ban nullptr, nincs inicializálva
  - van hang, de freezel, szerintem ez az audiopluginhost bug-ja

* megcsináltam a parameter attachment-et, de a value update nem működik
  - nem lehet a regisztrációkor megkapott pointert haszálni onChange callback-ben,
    seg faulthoz vezet. nem tudom egyelőre, hogy miért, de kell egy robosztusabb mechanizmus
  - segfault instancelist-ből kiválasztásnál (amikor már elv. ki van választva)

* parameter csatlakozás megvan. instance list nem updatel nagyon sok mindenre...

* okay, mostmár nem crashel a dtor, de még mindig nem frissül helyesen az instance list
  