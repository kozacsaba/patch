## DELETE THIS FILE BEFORE MERGING INTO DEVELOP

-----------------------------------------------

### Goals

- [ ] Add UIDs to instances
- [ ] register instances with UIDs
- [ ] presets __SHOULD NOT__ save or load IDs presets should only save / load
      parameters, except when loading a project...
- [ ] the core should keep three lists, that will contain all instances. one for
      transmitters, one for recievers, one for bystanders
- [ ] the core should keep a matrix of sends routing transmitters to recievers,
      that way it will be easy to look up connections for transmitters AND
      recievers
- [ ] every transmitter should have a list of possible instances to transmit to,
      and evey reciever should have a list of instances to recieve from. Next to
      each instance should be a dial that sets the level. These lists all refer
      to the same underlying matrix, owned by the core
