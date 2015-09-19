# Armi-Acco (Arduino MIFARE Access Control)

Access control system for Arduino using an [PN532](http://www.adafruit.com/products/364) to read Mifare Classic TAGs. 

## Installing

```
git submodule init
git submodule update
```

## How it works
Yo need to set a `Master TAG` that lets you manage `Acess TAGs`. The only way to do this is by setting `MASTER_TAG` constant on `NfcAccessContol/NfcAcessControl.h, line 4` to your own `TAG UUID`. Now you can upload the code to your board. 

The application has 3 different modes:

- **Normal mode**: Use any TAG except the `MASTER_TAG`. Checks if a TAG is authorized or not.
- **Master mode**: Use `MASTER_TAG`. After this mode is started, the next TAG will be added to the authorized list.
- **Clear mode**: Use `MASTER_TAG` and then use it again. Clears the authorized TAGs list.