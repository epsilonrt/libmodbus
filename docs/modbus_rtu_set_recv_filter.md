# modbus_rtu_set_recv_filter(3)

## Name

modbus_rtu_set_recv_filter - set reception filter flag of the context


## Synopsis

```c
int modbus_rtu_set_recv_filter(modbus_t *'ctx', int 'flag');*
```

## Description

The `modbus_rtu_set_recv_filter()` function shall set the reception filter flag 
of the `modbus_t` context by using the argument _flag_. By default, the boolean 
flag is set to `TRUE`. When the value _flag_ is set to `TRUE`, only messages to 
the address of the slave defined in the context and broadcast messages are 
returned by `modbus_receive`, the others are ignored. When the value _flag_ is 
set to `FALSE` all messages are returned by `modbus_receive`.

This function can only be used with a context using a RTU backend.

## Return value

The function shall return 0 if successful. Otherwise it shall return -1 and set errno.

## See also

- [modbus_receive](modbus_receive.md)  
- [modbus_rtu_get_recv_filter](modbus_rtu_get_recv_filter.md)  
