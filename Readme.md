# libicydb

libicydb is a library for maintaining and using program state that is synchronized to the harddrive during runtime. It is based around the concept of tables which come in a few different variants. An important part of this system is the table compiler. The table compiler can be used to generate a table with typed columns. 

It is called libicy, because freezing is something that can be done to preserve things. The goal of libicy is to store data on the disk. It is also called icy because it is meant to provide a basis for performant software development, efficiency being a high priority.

All the tables can be persisted to disk, but they can also be used in memory, this can be used in those situations where the data might not make sense when used between application starts.

One of the goals of this system is also to be able to share data between running programs, but at the moment this is a missing feature.


## Compiling
The whole system should be compilable on a default linux installation. Work has to be done to support windows as the memory mapped system there is a bit different

To compile:
```
make all
sudo make install
```

# Testing
To run tests, run the following code:
```
make all
make tests
```

