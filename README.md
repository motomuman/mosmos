## mosmos
Toy OS for learning purpose

## Demo
![audio_visualizer](https://github.com/motomuman/mosmos/blob/master/docs/mosmos_demo.gif?raw=true)

## Features
- x86_64 support
- Multitasking
- RTL8169 NIC driver
- Network stack (TCP, UDP, IP)
- Userland CLI application

## Todos
- Run on QEMU
- Better memory/page management
- Support fragmented IP packet
- Improve TCP stack
  - Handle reordering and options
  - Window control
  - Test with various cases (e.g. pkt loss, large delay)
- SMP support
- File system
- UEFI support


## mosmos image
|      |begin |end    |size|
|---   |---   |---    |--- |
|mbr   |0     |0x01ff |512B|
|boot  |0x200 |0x1ffff|8KB |
|kernel|0x2000|0x11fff|64KB|

## Memory map
|                             |begin     |end       |size|
|---                          |---       |---       |--- |
|mbr + boot                   |0x00007c00|0x00009bff|8KB |
|kernel(initial)              |0x00009c00|0x00019bff|64KB|
|page table for long mode     |0x00079000|0x00079fff|4KB |
|gdt table                    |0x00090000|0x00090fff|4KB |
|intrupption vec              |0x00091000|0x00092fff|8KB |
|kernel(relocated)            |0x00101000|0x00110fff|64KB|
|kstack for userapp(temporary)|          |0x00900000|    |
|free(managed by mem lib)     |0x00a00000|          |    |

## References
- General OS knowledge
  - [はじめて読む486](https://www.amazon.co.jp/dp/B00OCF5YUA/)
  - [30日でできる! OS自作入門](https://www.amazon.co.jp/dp/B00IR1HYI0/)
  - [作って理解するOS x86系コンピュータを動かす理論と実装](https://www.amazon.co.jp/dp/429710847X/)
  - [０から作るソフトウェア開発](http://softwaretechnique.jp/index.html)
  - [OSDev](https://wiki.osdev.org/Expanded_Main_Page)
- RTL8169 driver
  - [OSDev RTL8139](https://wiki.osdev.org/RTL8139)
  - [OSDev RTL8169](https://wiki.osdev.org/RTL8169)
  - [RTL8169 Datasheet](https://datasheetspdf.com/datasheet/RTL8169.html)
- 64bit/paging/context switch
  - https://github.com/drpnd/advos
- Implementation
  - https://github.com/matsud224/tinyos
  - https://github.com/Totsugekitai/minOSv2
  - https://github.com/szhou42/osdev
