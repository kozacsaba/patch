## DELETE THIS FILE BEFORE MERGING INTO DEVELOP

--------------------------------------------------------------------------------

### Goals

- [x] Add UUIDs to instances
- [x] register instances with UIDs
- [ ] UUIDs will be saved in the preset / StateInfo.
- [ ] on state info load, the uuid will be overwritten and all connecitons will
      be inherited
- [ ] connecitons should be stored in the preset and registered in the core on
      load. dangling connections are ignored
- [x] the core should keep three lists, that will contain all instances. one for
      transmitters, one for recievers, one for bystanders
- [x] the core should keep a matrix of sends routing transmitters to recievers,
      that way it will be easy to look up connections for transmitters AND
      recievers
- [x] every transmitter should have a list of possible instances to transmit to,
      and evey reciever should have a list of instances to recieve from. Next to
      each instance should be a dial that sets the level. These lists all refer
      to the same underlying matrix, owned by the core
- [ ] connection parameters should have a custom parameter struct / class that
      can manage and handle changes, attachments and change callbacks. make sure
      a callback is not called on a slider that initiated the change
