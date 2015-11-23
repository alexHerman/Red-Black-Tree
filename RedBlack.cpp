#include <iostream>
#include <fstream>
#include <string>

using namespace std;

struct RBnode
{
	RBnode();
	int  key_value;
	int  left_child;    // child file address
						// [record index, not byte address]
	int  right_child; 
	bool left_red;      // true if left child is red
	bool right_red;     // true if right child is red

	bool pad1;          // padding since compiler allocates two extra bytes
	bool pad2;          // padding since compiler allocates two extra bytes

	int location;		// location index
};

struct node
{
	int value;
	node *next;
};

RBnode::RBnode()
{
	right_child = -1;
	left_child = -1;
	right_red = false;
	left_red = false;
	pad1 = false;
	pad2 = false;
}

const int C_NULL = -1;

void test(fstream & binary);
bool checkarg(int argc);
bool fileopen(char *input, char *output);
int height(int index, fstream &binary);
void listFile(ifstream &text, fstream &binary);
bool readNode(int index, RBnode &shell, fstream &binary);
void writeNode(RBnode &shell, fstream &binary);
void insert(fstream &binary, int item);
void RBTreeRotate(RBnode &current, RBnode &parent, RBnode &grandparent, RBnode &greatgrandparent);
void editNode(RBnode &shell, fstream &binary);
void largest(int &max, int index, fstream &binary);
void smallest(int &min, int index, fstream &binary);

int main(int argc, char **argv)
{
	int h;
	int min;
	int max;
	int root;
	ifstream text;
	fstream binary;
	RBnode shell;

	if (!checkarg(argc))						//Check for valid argv
		return 1;

	if(!fileopen(argv[1], argv[2]))			//Check for valid files
		return 1;

	text.open(argv[1]);
	binary.open(argv[2], ios::in|ios::out|ios::binary|ios::trunc);

	listFile(text, binary);

	text.close();

	h = height(0, binary);
	smallest(min, 0, binary);
	largest(max, 0, binary);
	readNode(0, shell, binary);
	root = shell.key_value;
	binary.close();

	cout << "Number of nodes read: " << endl;
	cout << "Number of nodes written: " << endl;
	cout << "The height of this tree: " << h << endl;
	cout << "Root value: " << root << endl;
	cout << "Smallest value in leaf: " << min << endl;
	cout << "Largest value in leaf: " << max << endl;

	int i; 
	cin >> i;
	return 0;
}

bool checkarg(int argc)
{
	if(argc != 3)
	{
		cout << "Error: Invalid arguments." << endl;
		cout << "Usage: The first argument will be the name of a text file " <<
				"with a message that needs to be corrected.  The second " <<
				"argument will be the name of a binary file that contains the " <<
				"corrected message." << endl;
		cout << "Ex: c:\\>  prog3.exe  message.tgc  message.rst" << endl;
		return false;
	}
	else
		return true;
}

bool fileopen(char *input, char *output)
{
	ifstream fin;
	ofstream fout;

	fin.open(input);
	fout.open (output, ios::in|ios::out|ios::binary);

	if(!fin)
	{
		cout << "Error: Invalid input file." << endl;
		cout << "Usage: The first argument will be the name of a text file " <<
				"with a message that needs to be corrected.  The second " <<
				"argument will be the name of a text file that contains the " <<
				"corrected message." << endl;
		cout << "Ex: c:\\>  prog3.exe  message.tgc  message.rst" << endl;
		return false;
	}

	if(!fout)
	{
		cout << "Error: Invalid output file." << endl;
		cout << "Usage: The first argument will be the name of a text file " <<
				"with a message that needs to be corrected.  The second " <<
				"argument will be the name of a binary file that contains the " <<
				"corrected message." << endl;
		cout << "Ex: c:\\>  prog3.exe  message.tgc  message.rst" << endl;
		return false;
	}

	fin.close();
	fout.close();

	return true;
}

int height(int index, fstream &binary)
{
	int h_l;
	int h_r;
	int result;
	RBnode shell;

	if(!readNode(index, shell, binary))
	{
		return -1;
	}

	h_l = height(shell.left_child, binary);
	h_r = height(shell.right_child, binary);

	if(h_l > h_r)
		result = h_l;
	else
		result = h_r;

	return result + 1;
}

void insert(fstream &binary, int item)
{
	RBnode newNode;
	RBnode current;
	RBnode parent;
	RBnode grandparent;
	RBnode greatgrandparent;
	RBnode temp;
	bool looping = true;

	//Read root node
	readNode(0, current, binary);

	//Check if root node exists
	if (current.location == -1)
		looping = false;

	//Traverse tree until you find location that node should be inserted into
	while (looping == true)
	{
		if (item < current.key_value && current.left_child != -1)
		{
			greatgrandparent = grandparent;
			grandparent = parent;
			parent = current;
			readNode(current.left_child, current, binary);
		}
		else if (item > current.key_value && current.right_child != -1)
		{
			greatgrandparent = grandparent;
			grandparent = parent;
			parent = current;
			readNode(current.right_child, current, binary);
		}
		else
			looping = false;

		//Check for 4 nodes, and split them if they exist
		if (current.left_red == true && current.right_red == true)
		{
			current.left_red = false;
			current.right_red = false;

			if (current.key_value < parent.key_value)
				parent.left_red = true;
			else
				parent.right_red = true;

			//If splitting the 4 node creates 2 red nodes in a row, rotate the tree
			if (parent.key_value < grandparent.key_value && grandparent.left_red == true)
				RBTreeRotate(current, parent, grandparent, greatgrandparent);
			else if (parent.key_value > grandparent.key_value && grandparent.right_red == true)
				RBTreeRotate(current, parent, grandparent, greatgrandparent);
		}
	}

	//Assign values to new node
	newNode.key_value = item;
	binary.seekg(0, ios::end);
	newNode.location = int(binary.tellg() / 20);
	newNode.right_red = false;
	newNode.left_red = false;
	newNode.left_child = -1;
	newNode.right_child = -1;

	//Write new node to the file
	writeNode(newNode, binary);

	greatgrandparent = grandparent;
	grandparent = parent;
	parent = current;
	current = newNode;

	//Make new node a child of its parent
	if (current.key_value < parent.key_value)
	{
		parent.left_child = current.location;
		parent.left_red = true;
	}
	else
	{
		parent.right_child = current.location;
		parent.right_red = true;
	}

	//If there are two red nodes in a row, rotate the tree
	if ((grandparent.left_red == true && parent.key_value < grandparent.key_value) || (grandparent.right_red == true && parent.key_value > grandparent.key_value))
		RBTreeRotate(current, parent, grandparent, greatgrandparent);

	//Update all the nodes that were changed on the file
	if (greatgrandparent.location > -1)
		editNode(greatgrandparent, binary);
	if (grandparent.location > -1)
		editNode(grandparent, binary);
	if (parent.location > -1)
		editNode(parent, binary);
	editNode(current, binary);
}

void RBTreeRotate(RBnode &current, RBnode &parent, RBnode &grandparent, RBnode &greatgrandparent)
{
	//Check for the four different possibilities of rotations
	if (current.key_value < parent.key_value && current.key_value > grandparent.key_value)
	{
		//Assign the correct child to the great grandparent
		if (current.key_value > greatgrandparent.key_value)
		{
			greatgrandparent.right_child = current.location;
			greatgrandparent.right_red = false;
		}
		else
		{
			greatgrandparent.left_child = current.location;
			greatgrandparent.left_red = false;
		}

		//If the node needs to be switched with the root, change their locations
		if (grandparent.location == 0)
		{
			grandparent.location = current.location;
			current.location = 0;
		}

		//Change the children to be in the correct locations
		grandparent.right_child = current.left_child;
		grandparent.right_red = current.left_red;
		parent.left_child = current.right_child;
		parent.left_red = current.right_red;
		current.left_child = grandparent.location;
		current.left_red = false;
		current.right_child = parent.location;
		current.right_red = false;
	}
	else if (current.key_value < parent.key_value && current.key_value < grandparent.key_value)
	{
		if (parent.key_value > greatgrandparent.key_value)
		{
			greatgrandparent.right_child = parent.location;
			greatgrandparent.right_red = false;
		}
		else
		{
			greatgrandparent.left_child = parent.location;
			greatgrandparent.left_red = false;
		}
		if (grandparent.location == 0)
		{
			grandparent.location = parent.location;
			parent.location = 0;
		}
		grandparent.left_child = parent.right_child;
		grandparent.left_red = parent.right_red;
		parent.right_child = grandparent.location;
		parent.left_red = false;
		parent.right_red = false;
	}
	else if (current.key_value > parent.key_value && current.key_value > grandparent.key_value)
	{
		if (parent.key_value > greatgrandparent.key_value)
		{
			greatgrandparent.right_child = parent.location;
			greatgrandparent.right_red = false;
		}
		else
		{
			greatgrandparent.left_child = parent.location;
			greatgrandparent.left_red = false;
		}
		if (grandparent.location == 0)
		{
			grandparent.location = parent.location;
			parent.location = 0;
		}
		grandparent.right_child = parent.left_child;
		grandparent.right_red = parent.left_red;
		parent.left_child = grandparent.location;
		parent.left_red = false;
		parent.right_red = false;
	}
	else if (current.key_value > parent.key_value && current.key_value < grandparent.key_value)
	{
		if (current.key_value > greatgrandparent.key_value)
		{
			greatgrandparent.right_child = current.location;
			greatgrandparent.right_red = false;
		}
		else
		{
			greatgrandparent.left_child = current.location;
			greatgrandparent.left_red = false;
		}
		if (grandparent.location == 0)
		{
			grandparent.location = current.location;
			current.location = 0;
		}
		current.left_child = parent.location;
		current.left_red = false;
		current.right_child = grandparent.location;
		current.right_red = false;
		grandparent.left_child = current.right_child;
		grandparent.left_red = current.right_red;
		parent.right_child = current.left_child;
		parent.right_red = current.left_red;
	}
}

//write loop walking through list of input integers to insert into tree
//make it so list deletes itself after going through a node?

void listFile(ifstream &text, fstream &binary)
{
	int num;
	
	while(text >> num)
		insert(binary, num);
}

bool readNode(int index, RBnode &shell, fstream &binary)
{
	int location;

	binary.seekg(0, ios::end);

	location = binary.tellg();

	if (location == 0)
	{
		shell.left_child = -1;
		shell.right_child = -1;
		shell.location = -1;
		return false;
	}

	if(index == -1)
		return false;

	binary.seekg(index*20, ios::beg);
	binary.read((char *) &shell, 20);

	return true;
}

void writeNode(RBnode &shell, fstream &binary)
{
	binary.seekp(0, ios::end);
	binary.write((char *) &shell, 20);
}

void editNode(RBnode &shell, fstream &binary)
{
	binary.seekp(shell.location * 20, ios::beg);
	binary.write((char *)&shell, 20);
}

void test(fstream & binary)
{
	RBnode shell;

	for (int i = 0; i < 5; i++)
	{
		readNode(i, shell, binary);

		cout << "Index: " << shell.location << endl;
		cout << "Value: " << shell.key_value << endl;
		cout << "Left-Child: " << shell.left_child << endl;
		cout << "Left-Red: " << shell.left_red << endl;
		cout << "Right-Child: " << shell.right_child << endl;
		cout << "Right-Red: " << shell.right_red << endl << endl << endl;
	}
}

void smallest(int &min, int index, fstream &binary)
{
	RBnode shell;

	readNode(index, shell, binary);

	while (readNode(shell.left_child, shell, binary))
	{
		while (readNode(shell.left_child, shell, binary))
		{
		}
		readNode(shell.right_child, shell, binary);
	}

	min = shell.key_value;
}

void largest(int &max, int index, fstream &binary)
{
	RBnode shell;

	readNode(index, shell, binary);

	while (readNode(shell.right_child, shell, binary))
	{
		while (readNode(shell.right_child, shell, binary))
		{
		}
		readNode(shell.left_child, shell, binary);
	}

	max = shell.key_value;
}