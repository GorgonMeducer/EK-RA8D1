# MDK Template for EK-RA8D1

This is a **modified** MDK project template for EK-RA8D1. For the moment I writing this document (14 Dec 2023), the RA-generated MDK projects have some known limitations:

- Only supports **microLib** (this has some performance impact)
- Does NOT fully follow the standard scatter-loading and C-library startup procedure suggested by the CMSIS standard.
- DCache is disabled by default (this has a significant performance impact)
- ITCM and DTCM only accept contents that are placed in dedicated sections `.dtcm_data.*` and `.item_data.*`. Otherwise, BusFult will be triggered. 
- Numerous warnings are thrown out once **Link-Time-Optimization** is enabled, although they are safe to ignore. (This prevents users from allowing Arm Compiler 6 to do further optimization possible)



To buy time for the Renesas Team to solve the issue and to provide a template for evaluation purposes, we modified an RA-generated MDK project and patched the issues aforementioned.

### Feature

- Complaint with CMSIS standard
- DTCM and ITCM are available for random access without special procedures
- Supports both microlib and normal C library
- Free to use All available optimization levels (including -Omax and -Omin)
- Integrates [`perf_counter`](https://github.com/GorgonMeducer/perf_counter) and ready to run [`Coremark`](https://github.com/eembc/coremark)
- Ready to evaluate [`Arm-2D`](https://github.com/ARM-software/Arm-2D) and provides demos.
- Validated with [FSP 4.4.0](https://github.com/renesas/fsp/releases/tag/v4.4.0)



## How to Use

### How to Deploy

1. Download the latest [MDK](https://www.keil.com/download/product/) and install the [community license](https://www.keil.com/pr/article/1299.htm) if you haven't have a valid license yet. 

2. Clone the project with the following command line:

```shell
git clone https://github.com/GorgonMeducer/EK-RA8D1
```

3. Install required cmsis-packs
   - [FSP 4.4.0](https://github.com/renesas/fsp/releases/tag/v4.4.0) : MDK_Device_Packs
   - Pack Installer: `Perf_counter`

4. Enter the folder `EK-RA8D1` and open the project `RA8D1_MIPI_DSI_AC6.uvprojx`.

5. Compile.

If everything goese well, you should see the following output in Build Output window:

![BuildOutput](./document/picture/build_output.png) 

Congratulations!







