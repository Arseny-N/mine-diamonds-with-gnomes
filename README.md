#Mine Diamonds With Gnomes For Free ! #

New awesome console RPG. 
	
	
![Diamonds](http://www.bdlive.co.za/incoming/2014/08/07/johan-dippenaar-xxx/ALTERNATES/crop_400x250/Johan+Dippenaar+xxx)
## To install	

Do ```make main```

	
## To Run

Do ```./main ```

## Dependencies 

* make, gcc 
* ncurses 	

		
## Code Structure

* **Data Structures**
 - list.h    - linux linked list implementation
* **Graph related functions & types**
 - graphs.h graphs.c  - definition of ```struct graph``` alloc / free helper functions
 - print_graphs.h / print_graphs.c  - graph output functions. Output is done in Dot format.
 - vertex_list.h  - a buggy helper data structure
* **Misc files**
 - utils.h stddef.h types.h  - various helpers
 - main.c - _The Main File_
* **Random Number Generation**
 - random.c / random.h  - implementaion of some recepies from "The Numerical Recepies. Third Edition".
* **Build & Documentation**
 - README.md - README
 - Makefile  nice-display.mk   - makefile & stripe 




