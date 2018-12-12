# GEM OnlineDB Tools
## Outline
### Necessary parts
* `reader`
* `writer`
* `converter`
* `checker`
* `viewer`
* `blobber`
  * `pusher`
  * `puller`
  * `checker`

### Descriptions
#### reader
The `reader` is responsible for connecting to the DB and extracting the necessary information
* The connection could be bypassed by either
  * A dump (`sql`) file for the appropriate table
  * A filled template (`xml`) file for the appropriate table

#### writer
The `writer` is responsible for creating the template (`xml`) file for use in the `gemdbldr` tool to update the DB
* It's possible the SW could directly write to the/a DB, but do we want this?

#### converter
The `converter` should create a `c++` object(s) from the parsed DB/`xml` information that can then be used in the online SW.
* Objects could be defined for:
  * `VFAT`
    * `VFAT2`
    * `VFAT3`
  * `VFATChannel`
    * `VFAT2Channel`
    * `VFAT3Channel`
  * `OptoHybrid`
    * `OptoHybridV2`
    * `OptoHybridV3`
  * `AMC`
    * `CTP7`
    * `GLIB`
  * `AMC13`

#### checker
The `checker` tool is responsible for verifying the sanity of the template file, and may also be used to compare two files, a file against a database tag, a file against actual hardware, or actual hardware against a database tag
trying to understand if it would be at some point of interest to have "tagged" trim results, that could be toggled to when setting a configuration; in this case, having all dates fall under the `trim/` subdirectory would be less than ideal.

#### viewer
The `viewer` tool should be able to present the extracted DB information, allow modifications, and creation of new templates, whether through dumping the hardware, or manual interaction

#### blobber
The `blobber` tool should be able to create the configuration `blob` from the converted objects. The `blob` is used in the `CTP7` RAM for the `blaster` configuration method
##### pusher
The `pusher` is a tool that puts the `blob` in the appropriate location in the `CTP7` RAM
##### puller
The `puller` is a tool that can pull the `blob` from the appropriate location in the `CTP7` RAM
##### checker
The `checker` is a tool that can compare `blob`s
