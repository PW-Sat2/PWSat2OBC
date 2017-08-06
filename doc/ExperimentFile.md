# Experiment file format

## Introduction
The experiment file format has been created to produce unified and flexible file format for storing experiment data of PW-SAT2.

Main features of this file format are:
- Data should be organized into packets of fixed size, equal to payload length in communication frame.
- Losing any packet in transmission may not invalidate entire stream.
- File format should be transparent to data it contains and should allow to store any data.

The principles of format are similar to MPEG-TS multimedia file format, and some concepts are borrowed from there.  

This document sometimes uses RFC syntax for requirements.

## Structure

1.  Data are organized into **231 byte** packets.

    It is the size of `CorrelatedDownlinkFrame` used for downloading file.  
    It MUST be size available for file download in downlink frame.

2.  First byte in packet MUST BE **Synchronisation Byte** (set arbitrally as ASCII character 'G' / **0x47**).

    It marks start of packet and ensures that if part of packet will be lost in transmission, it is possible to restore synchronisation with data stream.  
    The *Synchronisation Byte* value has no known meaning. It is just... borrowed from MPEG-TS standard. ;)

3.  One or many **Data Blocks**.  

    The _Data Blocks_ are simple key-value pairs consisting of

       - **PID** (Packet Identifier) - key that is unique identifier for data block after it.  
       - **Data** - variable length array of real data inserted into file.

    The ExperimentFile doesn't have any information to decode length or content of _Data_.  
    The length of _Data_ field SHOULD be fixed for given PID.  
    If it is impossible then there MUST be additional code storing and/or retrieving length of data.  
    The position of next _Data block_ MUST be known or computable to demultiplexer.

4.  There is one specially defined _Data Block_: **Timestamp**. It is:

       - **PID::Timestamp**  
       - 8 bytes of Timestamp data. It SHOULD represent Mission Time in milliseconds.

    _Timestamp_ MAY be first _Data Block_ inside packet (and it is automatically created if ExperimentFile class is provided with Time Provider) but it also MAY be placed in any position in payload - it is just normal _Data Block_.

5.  There is special **PID::Continuation** (**0xFE**). It indicates that next _Data Block_ is continuation of previous frame, not the new data frame.  

    This is designed for data that doesn't fit into one downlink frame and splitting them by PID-s would create confusion if it is new data block or still the previous one.
    The PID before and after _PID::Continuation_ MUST be equal (other data MUST NOT be entered between such splitted _Data Block_)

6.  Packet SHOULD end with **Padding Block**.

    This is 0 - 231 bytes of dummy data filling packet.  
    It allows that the packet split is not in the middle of data block.

    Padding block consists of:

       - **PID::Padding** (**0xFF**)
       - **Padding Data** - zero or more **0xFF** bytes, until the end of packet. 

    There MUST NOT be real data after padding block inside one packet.  
    _PID::Padding_ SHOULD be last number allowed by PID size.  
    _Padding Data_ MUST be one byte and 0xFF.  
    _PID::Padding_ and _Padding Data_ SHOULD NOT be interpreted as the same entity even if they have the same value. PID may change size in the future, data will be always one byte.

For standarisation purposes the extension of ExperimentFile SHOULD BE **".pwts"** (**PW**-Sat2 **T**ransport **S**tream) as oposed to strictly binary DAT file formats.

## Writing

### Data blocks smaller than _Packet Length_

1. Create new frame
2. Write _Synchronisation Byte_
3. OPTIONAL Write _Timestamp Data Block_
4. While there is empty space  
    1. Write PID of inserted data
    2. Write _Data_
5. Write _Padding Block_ (it can have 0 length)

### Data blocks larger than _Packet Length_

1. Create new frame
2. Write _Synchronisation Byte_
3. OPTIONAL Write _Timestamp Data Block_
4. Write PID of inserted data
5. Write _Data_ until the end of frame
6. Start new frame
7. Write _Synchronisation Byte_
8. OPTIONAL Write _Timestamp Data Block_
9. Write _ID::Continuation_ 
10. Write PID of inserted data
11. Write the rest of _Data_ until the end of frame
12. If there is _Data_ remaining, go to 6.
13. Fill remaining frame space with _Padding Block_ OR go to 4.

## Reading

There is no need of going back during reading. Data already read can be forgotten. Read advances reading pointer.

1.  Load _Packet Length_ of data into buffer
2.  Check that first byte is _Synchronisation Byte_
    - if not, move frame ahead byte by byte until there is _Synchronisation Byte_ AND another _Synchronisation Byte_ is _Packet Length_ later.
3.  Decode PID
    1. If this is regular _Data Block_ PID
        1. Get from decoder responsible for this PID length of data it needs
        2. Provide that number of bytes to decoder
            - If number of bytes decoder asks for is greater than _Available Length_, then store data remaining in buffer and go to 1.
        3. Goto Main 3.
    2. If it is_PID::Continuation_
        1. Verify that next PID is equal to previous. If not, ignore BOTH data - previous and current.
        2. Append remaining data into buffer and provide to decoder if total length is equal to length expecting by decoder.
        3. If remaining data is less than _Available Length_, then go to Main 1.
        3. Else Goto Main 3.
    3. If it is _Padding Block_ then ignore the rest of data in buffer
4.  Go to 1 until there are data left.