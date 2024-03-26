| Name  | Class Driver | Required Interface | Interface ID | Required endpoint host - client | End point ID        | Endpoint packet size (bytes) | End point FIFO size (Pkt size / 16) |
|-------|--------------|--------------------|--------------|---------------------------------|---------------------|------------------------------|-------------------------------------|
| Frasy | VCP          | Control interface  | 0            | Out - Receive / VCP             | Not required / 0x81 | NA / 16                      | NA / 1                              |
| Frasy | VCP          | Data interface     | 1            | Out - Receive / In - Transmit   | 0x02 / 0x82         | 64 / 64                      | RX_FIFO / 4                         |
| Debug | VCP          | Control interface  | 2            | Out - Receive / VCP             | Not required / 0x83 | NA / 16                      | NA / 1                              |
| Debug | VCP          | Data interface     | 3            | Out - Receive / In - Transmit   | 0x04 / 0x84         | 64 / 64                      | RX_FIFO / 4                         |
