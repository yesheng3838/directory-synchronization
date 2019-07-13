# Directory Synchronization

Directory synchronization application within a local GNU/Linux system. The application will read the configuration file called _**sinco.conf**_ that reside in the user's _**HOME**_ directory. 
In this file (which will be created by hand before the execution of the code) there should be three lines of text indicating the address of the _**source directory**_, the address of the _**destination directory**_ and the _**path dynamic library**_ of the directories respectively. If the source directory does not exist, the program will report an error to the user and end. If the destination directory does not exist, the program will create it (call mkdir system).

## Requeriments
- Add PATH of _"sinco.conf"_ in main() of sinco.c file.

## Get and Execute

```sh
$ git clone https://github.com/igp7/Directory-Synchronization.git
$ make
$ ./sinco.out
```
## Contributing
Changes and improvements are more than welcome! Feel free to fork and open a pull request. Please if you can, please make sure the code fully works before sending the pull request, as that will help speed up the process.

## License
This code is licensed under the [GPL-2.0 License](https://github.com/igp7/Directory-Synchronization/blob/master/LICENSE).
