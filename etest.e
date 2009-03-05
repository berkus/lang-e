using list from system.collections;

// problem: template specification has type: var list inverted from the
// normal way. looks weird. maybe [hashable -> T] ? [hashable: T] looks 
// disorienting.

// variables are never public in class
// attaching a set/get methods to them makes them accessible
// get/set methods may have modifiers limiting scope.
// get/set is chosen with best matching scope. if no matching scope found,
// the global (public) one is used.
// (){} is anonymous function, [](){} is anonymous template function.
// [types](params){body}

class my_class[T: hashable]
{
	list[my_class]: l; // private by default
	int: var {
	    get = (){ return var; } // public
	    friend set = (int: v){ var = v; } // friend
	    set = (int: v){ if (v > var) var = v; } // public
	}

	int: function1(int: a, b, c, d)
	{
	    return 0;
	}
	
	// template function declaration
	int: function2[T1: my_class, T2: list](T1: a, b, T2: l);
}


my_class[string]: instance;
