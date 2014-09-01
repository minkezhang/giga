A performance test can be invoked by tuning the indicated parameters in `tests/performance.cc` and run by invoking:

```bash
make PERFORMANCE=true test
```

A sample output is included below:

```
      trial | tag |  R (%) |  W (%) |  I (%) |  E (%) | tput (B/us) |    lat (us) |    file (B) |    data (B) |       cache |    init (B) |     max (B) |   N |    miss (%)
============================================================================================================================================================================
          1 | Rsq | 100.00 |   0.00 |   0.00 |   0.00 |       65.56 |       31.64 |    16384000 |     2074.03 |         100 |        2048 |        3072 |   1 |      100.00
          2 | Rsq | 100.00 |   0.00 |   0.00 |   0.00 |       15.03 |      138.00 |    16384000 |     2074.03 |         100 |        2048 |        3072 |   2 |       97.20
          3 | Rsq | 100.00 |   0.00 |   0.00 |   0.00 |        7.69 |      269.61 |    16384000 |     2074.03 |         100 |        2048 |        3072 |   3 |       96.26
          4 | Rsq | 100.00 |   0.00 |   0.00 |   0.00 |        5.07 |      409.07 |    16384000 |     2074.03 |         100 |        2048 |        3072 |   4 |       95.72
          5 | Wsq |   0.00 | 100.00 |   0.00 |   0.00 |       19.92 |      104.10 |    16384000 |     2074.03 |         100 |        2048 |        3072 |   1 |       38.90
          6 | Wsq |   0.00 | 100.00 |   0.00 |   0.00 |        5.79 |      357.97 |    16384000 |     2074.03 |         100 |        2048 |        3072 |   2 |       37.83
          7 | Wsq |   0.00 | 100.00 |   0.00 |   0.00 |        3.35 |      618.32 |    16384000 |     2074.03 |         100 |        2048 |        3072 |   3 |       37.46
          8 | Wsq |   0.00 | 100.00 |   0.00 |   0.00 |        2.33 |      892.05 |    16384000 |     2074.03 |         100 |        2048 |        3072 |   4 |       37.03
...
```

* `R`|`W`|`I`|`E` -- the percentage of read / write / insert / erase calls being made
* `tput` -- the throughput as seen by **each client** -- the effective throughput is `N * tput`
* `file` -- the **initial** file size -- final file size may be vastly different in the case of insert and delete calls
* `data` -- the average amount of data being processed per transaction
* `cache` -- the number of pages kept in memory
* `init` -- initial page size
* `max` -- max page size before the data is split into two virtual pages
* `N` -- the number of clients writing to the file concurrently

Note that we are writing an average of about half a kilobyte into the file -- this is representing **incremental** changes to a file -- as we increase the average data, 
as well as the characteristic page size of the cache, we will be able to increase the throughput dramatically.

As performance needs may vary, the cache can be configured as an optional input into the `File` constructor -- cf. `include/src/file.h` to see the defaults and for how 
to invoke the optional `Config` argument.
