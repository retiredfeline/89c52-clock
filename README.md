# 89C52 based digital clock

Firmware files for 89C52 digital clock using a DS3231 RTC board.

It uses the [Protothreads library](http://dunkels.com/adam/pt/) by Adam Dunkels. You should adjust the -I option to the sdcc compiler to where the include files are installed

## Versioning

August 2020

## Notes

A local stc89.h include file is used due to a syntax error in the SDCC supplied one which may have been fixed in recent releases. If this is the case then you can change "stc89.h" to <stc89.h>. Otherwise it's easy to discover the line with the syntax error by trying to compile the package with the supplied include file.

## Authors

* **Ken Yap**

## License

See the [LICENSE](LICENSE.md) file for license rights and limitations (MIT).
