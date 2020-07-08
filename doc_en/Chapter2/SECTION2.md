# 2.2 Root (JSON)

**hipshook.json** is the root JSON file which contains the number identifiers of all the other JSON files. Its format looks like this:

```C++
{
    "Name": "hipshook",
    "Version": "1.0.0.0",
    "CreateDate": "24//12//2019 2:24PM",
    "Description": "Creating hooks and signature format config information head file",
    "Sigs": [
        104,
        105,
        108
    ],
    "Coms": [
        106
    ],
    "Flags": [
        107
    ],
    "Wmis": [
        109
    ]
}
```


Below explains each key:

* _Name_: the internal name of the root JSON file
* _Version_: the current release version number
* _CreateDate_: the date of release
* _Description_: a simple description
* _Sigs_: an array that contains the resource numbers of API JSON files in the resource directory
* _Flags_: an array that contains the resource numbers of Type JSON files in the resource directory
* _Wmis_: an array that contains the resource numbers of WMI JSON files in the resource directory
* _Coms_: an array that contains the resource numbers of COM JSON files in the resource directory



------

