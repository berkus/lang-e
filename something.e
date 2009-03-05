class List [T -> Hashable] inherit A redefine a,b,c end, C redefine y end
	
end

----

class List [T -> Hashable] : A <a,b,c>, C <y>
{
}

class ItemList : List[Item]; // specialization?

ItemList itemList;

----

proc Something(i: Int); external Cpp Something(Int) mangled gcc;
