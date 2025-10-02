# DELETE THIS FILE BEFORE MERGE

* no more runtime errors, but the instance list on the gui shows instances of the same Mode instead
  of possible connections...
  - fixed

* nincs hang
  - véletlenül eddit a transmitterekhez pusholtam a sample-eket, amiket a transmitterek küldenek...
  - átmozgattam a reciever loop-ba, és így crashel a plugin
  - CircularBuffer a Core-ban nullptr, nincs inicializálva
  - van hang, de freezel, szerintem ez az audiopluginhost bug-ja
  