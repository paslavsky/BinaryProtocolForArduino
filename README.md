# Binary Protocol for Arduino (BPA)

## Binary message format
```
<start-byte><device-id><message-id><payload-length>[<payload><hash>]

byte-mask: SDML[(P*)HH]
```

### Start byte
| ASCII code | Symbol | Description                                          |
|:----------:|:------:|------------------------------------------------------|
|     0      |        | Undefined                                            |
|            |        |                                                      |
|     48     |  `0`   | Input message (Protocol version 1)                   |
|    ...     |  ...   | ...                                                  |
|     57     |  `9`   | Input message (Protocol version 10)                  |
|            |        |                                                      |
|     65     |  `A`   | Confirmation - input message was received and parsed |
|    ...     |  ...   | ...                                                  |
|     70     |  `F`   | Response - format incorrect                          |
|    ...     |  ...   | ...                                                  |
|     72     |  `H`   | Response - hash error                                |
|    ...     |  ...   | ...                                                  |
|     82     |  `R`   | Response - reject input message                      |
|    ...     |  ...   | ...                                                  |
|     90     |  `Z`   | `Reserved`                                           |
|            |        |                                                      |
|     42     |  `*`   | Handshake init                                       |
|     43     |  `+`   | Handshake response                                   |
|     46     |  `.`   | Handshake complete                                   |
|            |        |                                                      |
|    126     |  `~`   | Disconnect (no response needed)                      |

### Device ID
A one-byte value that uniquely identifies the client device.

### Message ID
A byte value that allows a message to be separated from another in a short period of time. BPA doesn't store history, so it's enough to distinguish two messages semintaniusly.

### Length
A byte value indicating the number of bytes we should read as a `payload`.

### Payload
Up to 256 byte array of the message.

### Hash 
Two byte values representing the Fowler-Noll-Vo (FNL) hash of the previous bytes. 
Here is an example of the hash function:
```c
uint16_t fnv1a_hash16(uint8_t* bytes, size_t length) {
    uint16_t hash = (uint16_t)0x97;
    for (size_t i = 0; i < length; i++)
    {
        hash ^= bytes[i];
        hash *= 0xA1;
    }
    return hash;
}
```

## UDP Tunneling
_TBD_