## DELETE THIS FILE BEFORE MERGING INTO DEVELOP

--------------------------------------------------------------------------------

### Goals

- [ ] Add UUIDs to instances
- [ ] register instances with UIDs
- [ ] UUIDs will be saved in the preset / StateInfo.
- [ ] on state info load, the uuid will be overwritten and all connecitons will
      be inherited
- [ ] connecitons should be stored in the preset and registered in the core on
      load. dangling connections are ignored
- [ ] the core should keep three lists, that will contain all instances. one for
      transmitters, one for recievers, one for bystanders
- [ ] the core should keep a matrix of sends routing transmitters to recievers,
      that way it will be easy to look up connections for transmitters AND
      recievers
- [ ] every transmitter should have a list of possible instances to transmit to,
      and evey reciever should have a list of instances to recieve from. Next to
      each instance should be a dial that sets the level. These lists all refer
      to the same underlying matrix, owned by the core
