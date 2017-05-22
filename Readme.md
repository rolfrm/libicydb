# libIcyDB

libIcyDB is a library for maintaining and using program state that is synchronized to the harddrive during runtime. It is based around the concept of tables which come in a few different variants. An important part of this system is the table compiler. The table compiler can be used to generate a table with typed columns. 

It is called libicy, because freezing is something that can be done to preserve things. The goal of libicy is to store data on the disk. It is also called icy because it is meant to provide a basis for performant software development, efficiency being a high priority.

All the tables can be persisted to disk, but they can also be used in memory, this can be used in those situations where the data might not make sense when used between application starts.

One of the goals of this system is also to be able to share data between running programs, but at the moment this is a missing feature.


## Compiling

- ** Note: ** this does not yet work.

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

## usage:

Currently it only compiles with GCC on linux. add -licydb to your gcc arguments.

to create a new table use the following syntax:

```./compile_table columnName1:columnType1 columnName2:columnType2 ...```

The first column is the index column. The following example creates an int to float lookup table.
```./compile_table int_to_float key:int value:float```

This will create a file called int_to_float.c and int_to_float.h. Include these where needed. They should each be able to compile on their own.

The following is a short

### Tables
a table is a set of columns (arrays), indexed by a key column. The key column is sorted so that lookups can be efficiently performed. Single element lookups are not extremely effective, the true efficiency arrises from doing lookups with multiple keys at the same time.

### Multi Tables
multi-tables are tables where the same key can map to multiple values. To enable multi table support, set 'is_multi_table' to true on the table. Some of the generated functions supports looking many up at the same time.

### Index Tables
Index tables are value only-tables. This means that there is no key, except of the index in the table. This means that elements cannot be removed from the table, as that would mess the indexes up. So to accomondate removing elements, they are pushed to a 'free' table. Having a free table means that the next time an element is allocated, it will start by trying there.

An 'index table sequence' is a range of elements, starting at an offset, being N elements long.

The indexes will stay the same, but pointers pointing into the table are invalid if any insert happens after the pointer is calculated.


