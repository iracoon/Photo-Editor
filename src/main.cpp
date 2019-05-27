#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

struct imageData
{
	char  idLength;
	char  colorMapType;
	char  dataTypeCode;
	short colorMapOrigin;
	short colorMapLength;
	char  colorMapDepth;
	short xOrigin;
	short yOrigin;
	short width;
	short height;
	char  bitsPerPixel;
	char  imageDescriptor;

	struct Pixel
	{
		unsigned char r = 0;
		unsigned char g = 0;
		unsigned char b = 0;
	};
	vector<Pixel> pixels;
};

///file operations
imageData readFile(string fileName)
{
	ifstream file(fileName, ios::in | ios::binary);

	imageData tmp;
	file.read(&tmp.idLength, sizeof(tmp.idLength));
	file.read(&tmp.colorMapType, sizeof(tmp.colorMapType));
	file.read(&tmp.dataTypeCode, sizeof(tmp.dataTypeCode));
	file.read((char*)(&tmp.colorMapOrigin), sizeof(tmp.colorMapOrigin));
	file.read((char*)(&tmp.colorMapLength), sizeof(tmp.colorMapLength));
	file.read(&tmp.colorMapDepth, sizeof(tmp.colorMapDepth));
	file.read((char*)(&tmp.xOrigin), sizeof(tmp.xOrigin));
	file.read((char*)(&tmp.yOrigin), sizeof(tmp.yOrigin));
	file.read((char*)(&tmp.width), sizeof(tmp.width));
	file.read((char*)(&tmp.height), sizeof(tmp.height));
	file.read(&tmp.bitsPerPixel, sizeof(tmp.bitsPerPixel));
	file.read(&tmp.imageDescriptor, sizeof(tmp.imageDescriptor));

	streampos size;
	char *memblock;

	if (file.is_open())
	{
		size = tmp.height*tmp.width*3;
		memblock = new char[size];
		file.seekg(18, ios::beg);
		file.read(memblock, size);
		file.close();

		int counter = 0;
		imageData::Pixel t;
		tmp.pixels.reserve(200000);
		char blue, green, red;
		for (int i = 0; i < size/3; i++)
		{
			t.b = memblock[counter++];
			t.g = memblock[counter++];
			t.r = memblock[counter++];
			tmp.pixels.push_back(t);
		}
		delete[] memblock;
	}
	return tmp;
}

///helper functions
void writeHeader(fstream &file, imageData tmp)
{
	file.write(&tmp.idLength, sizeof(tmp.idLength));
	file.write(&tmp.colorMapType, sizeof(tmp.colorMapType));
	file.write(&tmp.dataTypeCode, sizeof(tmp.dataTypeCode));
	file.write((char*)(&tmp.colorMapOrigin), sizeof(tmp.colorMapOrigin));
	file.write((char*)(&tmp.colorMapLength), sizeof(tmp.colorMapLength));
	file.write(&tmp.colorMapDepth, sizeof(tmp.colorMapDepth));
	file.write((char*)(&tmp.xOrigin), sizeof(tmp.xOrigin));
	file.write((char*)(&tmp.yOrigin), sizeof(tmp.yOrigin));
	file.write((char*)(&tmp.width), sizeof(tmp.width));
	file.write((char*)(&tmp.height), sizeof(tmp.height));
	file.write(&tmp.bitsPerPixel, sizeof(tmp.bitsPerPixel));
	file.write(&tmp.imageDescriptor, sizeof(tmp.imageDescriptor));

}

void footer(int &bc, int &gc, int &rc, fstream &file)
{
	if (bc < 0) { bc = 0; }
	if (gc < 0) { gc = 0; }
	if (rc < 0) { rc = 0; }
	if (bc > 255) { bc = 255; }
	if (gc > 255) { gc = 255; }
	if (rc > 255) { rc = 255; }
	file.write((char*)&bc, 1);
	file.write((char*)&gc, 1);
	file.write((char*)&rc, 1);
}

///manipulations
void subtract(imageData &top, imageData &bottom, string fileName)
{
	fstream file(fileName, ios::out | ios::binary);
	writeHeader(file, top);

	float ba, bb, ga, gb, ra, rb;
	int bc, gc, rc;
	for (int i = 0; i < top.height*top.width; i++)
	{
		ba = (float)top.pixels.at(i).b;
		bb = (float)bottom.pixels.at(i).b;
		bc = bb - ba + .5f;

		ga = (float)top.pixels.at(i).g;
		gb = (float)bottom.pixels.at(i).g;
		gc = gb - ga + .5f;

		ra = (float)top.pixels.at(i).r;
		rb = (float)bottom.pixels.at(i).r;
		rc = rb + .5f;
		footer(bc, gc, rc, file);
	}
	file.close();
}

void multiply(imageData &one, imageData &two, string fileName)
{
	fstream file(fileName, ios::out | ios::binary);
	writeHeader(file, one);

	for (int i = 0; i < one.height*one.width; i++)
	{
		float ba = (float)one.pixels.at(i).b;
		float bb = (float)two.pixels.at(i).b;
		int bc = (ba*bb) / 255 + .5f;

		float ga = (float)one.pixels.at(i).g;
		float gb = (float)two.pixels.at(i).g;
		int gc = (ga*gb) / 255 + .5f;

		float ra = (float)one.pixels.at(i).r;
		float rb = (float)two.pixels.at(i).r;
		int rc = (ra*rb) / 255 + .5f;
		footer(bc, gc, rc, file);
	}
	file.close();
}

void screen(imageData &top, imageData &bottom, string fileName)
{
	fstream file(fileName, ios::out | ios::binary);
	writeHeader(file, bottom);

	float ba, bb, ga, gb, ra, rb;
	int bc, gc, rc;
	for (int i = 0; i < top.height*top.width; i++)
	{
		ba = (float)top.pixels.at(i).b / 255;
		bb = (float)bottom.pixels.at(i).b / 255;
		bc = (1 - (1 - ba)*(1 - bb)) * 255 + .5f;

		ga = (float)top.pixels.at(i).g / 255;
		gb = (float)bottom.pixels.at(i).g / 255;
		gc = (1 - (1 - ga)*(1 - gb)) * 255 + .5f;

		ra = (float)top.pixels.at(i).r / 255;
		rb = (float)bottom.pixels.at(i).r / 255;
		rc = (1 - (1 - ra)*(1 - rb)) * 255 + .5f;
		footer(bc, gc, rc, file);
	}
	file.close();
}

void overlay(imageData &top, imageData &bottom, string fileName)
{
	fstream file(fileName, ios::out | ios::binary);
	writeHeader(file, bottom);

	float ba, bb, ga, gb, ra, rb;
	int bc, gc, rc;
	for (int i = 0; i < top.height*top.width; i++)
	{
		ba = (float)top.pixels.at(i).b / 255;
		bb = (float)bottom.pixels.at(i).b / 255;
		bc = (1 - 2 * (1 - ba)*(1 - bb)) * 255 + .5f;
		if (bb <= 0.5) { bc = 2 * ba*bb * 255 + .5f; }

		ga = (float)top.pixels.at(i).g / 255;
		gb = (float)bottom.pixels.at(i).g / 255;
		gc = (1 - 2 * (1 - ga)*(1 - gb)) * 255 + .5f;
		if (gb <= 0.5) { gc = 2 * ga*gb * 255 + .5f; }


		ra = (float)top.pixels.at(i).r / 255;
		rb = (float)bottom.pixels.at(i).r / 255;
		rc = (1 - 2 * (1 - ra)*(1 - rb)) * 255 + .5f;
		if (rb <= 0.5) { rc = 2 * ra*rb * 255 + .5f; }

		footer(bc, gc, rc, file);
	}
	file.close();
}

void add(imageData &one, string fileName, int blue, int green, int red)
{
	fstream file(fileName, ios::out | ios::binary);
	writeHeader(file, one);

	float ba, ga, ra;
	int bc, gc, rc;
	for (int i = 0; i < one.height*one.width; i++)
	{
		ba = (float)one.pixels.at(i).b;
		bc = ba + blue + .5f;

		ga = (float)one.pixels.at(i).g;
		gc = ga + green + .5f;

		ra = (float)one.pixels.at(i).r;
		rc = ra + red + .5f;

		footer(bc, gc, rc, file);
	}
}

void scale(imageData &one, string fileName, int blue, int green, int red)
{
	fstream file(fileName, ios::out | ios::binary);
	writeHeader(file, one);

	float ba, ga, ra;
	int bc, gc, rc;
	for (int i = 0; i < one.height*one.width; i++)
	{
		ba = (float)one.pixels.at(i).b / 255;
		bc = ba * blue * 255 + .5f;

		ga = (float)one.pixels.at(i).g / 255;
		gc = ga * green * 255 + .5f;

		ra = (float)one.pixels.at(i).r / 255;
		rc = ra * red * 255 + .5f;

		footer(bc, gc, rc, file);
	}
}

void seperate(imageData &original, string blueFile, string greenFile, string redFile)
{
	fstream file1(blueFile, ios::out | ios::binary);
	writeHeader(file1, original);

	fstream file2(greenFile, ios::out | ios::binary);
	writeHeader(file2, original);

	fstream file3(redFile, ios::out | ios::binary);
	writeHeader(file3, original);

	int bc, gc, rc;
	for (int i = 0; i < original.height*original.width; i++)
	{
		bc = (float)original.pixels.at(i).b + 0.5f;
		gc = (float)original.pixels.at(i).b + 0.5f;
		rc = (float)original.pixels.at(i).b + 0.5f;
		footer(bc, gc, rc, file1);

		bc = (float)original.pixels.at(i).g + 0.5f;
		gc = (float)original.pixels.at(i).g + 0.5f;
		rc = (float)original.pixels.at(i).g + 0.5f;
		footer(bc, gc, rc, file2);

		bc = (float)original.pixels.at(i).r + 0.5f;
		gc = (float)original.pixels.at(i).r + 0.5f;
		rc = (float)original.pixels.at(i).r + 0.5f;
		footer(bc, gc, rc, file3);
	}
}

void combine(imageData &blue, imageData &green, imageData &red, string fileName)
{
	fstream file(fileName, ios::out | ios::binary);
	writeHeader(file, blue);

	int bc, gc, rc;
	for (int i = 0; i < blue.height*blue.width; i++)
	{
		bc = (float)blue.pixels.at(i).b + 0.5f;
		gc = (float)green.pixels.at(i).g + 0.5f;
		rc = (float)red.pixels.at(i).r + 0.5f;
		footer(bc, gc, rc, file);
	}
}

void flip(imageData &one, string fileName)
{
	fstream file(fileName, ios::out | ios::binary);
	writeHeader(file, one);

	int bc, gc, rc;
	for (int i = one.height*one.width - 1; i >= 0; i--)
	{
		bc = (float)one.pixels.at(i).b + 0.5f;
		gc = (float)one.pixels.at(i).g + 0.5f;
		rc = (float)one.pixels.at(i).r + 0.5f;
		footer(bc, gc, rc, file);
	}
}

void four(imageData &bottomLeft, imageData &bottomRight, imageData &topLeft, imageData &topRight, string fileName)
{
	short width = bottomLeft.width * 2;
	short height = bottomLeft.height * 2;
	int bc, gc, rc;

	fstream file(fileName, ios::out | ios::binary);
	file.write(&bottomLeft.idLength, sizeof(bottomLeft.idLength));
	file.write(&bottomLeft.colorMapType, sizeof(bottomLeft.colorMapType));
	file.write(&bottomLeft.dataTypeCode, sizeof(bottomLeft.dataTypeCode));
	file.write((char*)(&bottomLeft.colorMapOrigin), sizeof(bottomLeft.colorMapOrigin));
	file.write((char*)(&bottomLeft.colorMapLength), sizeof(bottomLeft.colorMapLength));
	file.write(&bottomLeft.colorMapDepth, sizeof(bottomLeft.colorMapDepth));
	file.write((char*)(&bottomLeft.xOrigin), sizeof(bottomLeft.xOrigin));
	file.write((char*)(&bottomLeft.yOrigin), sizeof(bottomLeft.yOrigin));
	file.write((char*)(&width), sizeof(bottomLeft.width));
	file.write((char*)(&height), sizeof(bottomLeft.height));
	file.write(&bottomLeft.bitsPerPixel, sizeof(bottomLeft.bitsPerPixel));
	file.write(&bottomLeft.imageDescriptor, sizeof(bottomLeft.imageDescriptor));

	int bl = 0;
	int br = 0;
	int tl = 0;
	int tr = 0;
	for (int i = 0; i < bottomLeft.height; i++)
	{
		for (int i = 0; i < bottomLeft.width; i++)
		{
			bc = (float)bottomLeft.pixels.at(bl).b + 0.5f;
			gc = (float)bottomLeft.pixels.at(bl).g + 0.5f;
			rc = (float)bottomLeft.pixels.at(bl).r + 0.5f;
			bl++;
			footer(bc, gc, rc, file);
		}
		for (int i = 0; i < bottomRight.width; i++)
		{
			bc = (float)bottomRight.pixels.at(br).b + 0.5f;
			gc = (float)bottomRight.pixels.at(br).g + 0.5f;
			rc = (float)bottomRight.pixels.at(br).r + 0.5f;
			br++;
			footer(bc, gc, rc, file);
		}
	}
	for (int i = 0; i < topLeft.height; i++)
	{
		for (int i = 0; i < topLeft.width; i++)
		{
			bc = (float)topLeft.pixels.at(tl).b + 0.5f;
			gc = (float)topLeft.pixels.at(tl).g + 0.5f;
			rc = (float)topLeft.pixels.at(tl).r + 0.5f;
			tl++;
			footer(bc, gc, rc, file);
		}
		for (int i = 0; i < topRight.width; i++)
		{
			bc = (float)topRight.pixels.at(tr).b + 0.5f;
			gc = (float)topRight.pixels.at(tr).g + 0.5f;
			rc = (float)topRight.pixels.at(tr).r + 0.5f;
			tr++;
			footer(bc, gc, rc, file);
		}
	}
}

///test
template<typename InputIterator1, typename InputIterator2>
bool range_equal(InputIterator1 first1, InputIterator1 last1, InputIterator2 first2, InputIterator2 last2)
{
	while (first1 != last1 && first2 != last2)
	{
		if (*first1 != *first2) return false;
		++first1;
		++first2;
	}
	return (first1 == last1) && (first2 == last2);
}

void compareTest(const std::string& filename1, const std::string& filename2)
{
	std::ifstream file1(filename1);
	std::ifstream file2(filename2);
	std::istreambuf_iterator<char> begin1(file1);
	std::istreambuf_iterator<char> begin2(file2);
	std::istreambuf_iterator<char> end;

	if (range_equal(begin1, end, begin2, end)) 
	{ 
		cout << "Test Passed" << endl; 
	}
	else 
	{ 
		cout << "Test Failed" << endl; 
	}
}

void test1()
{
	cout << "Test #1" << endl;
	imageData one = readFile("input/layer1.tga");
	imageData two = readFile("input/pattern1.tga");
	multiply(one, two, "output/part1.tga");
	compareTest("output/part1.tga", "examples/EXAMPLE_part1.tga");
}

void test2()
{
	cout << "Test #2" << endl;
	imageData top = readFile("input/layer2.tga");
	imageData bottom = readFile("input/car.tga");
	subtract(top, bottom, "output/part2.tga");
	compareTest("output/part2.tga", "examples/EXAMPLE_part2.tga");
}

void test3()
{
	cout << "Test #3" << endl;
	imageData one = readFile("input/layer1.tga");
	imageData two = readFile("input/pattern2.tga");
	multiply(one, two, "output/temporary.tga");
	imageData bottom = readFile("output/temporary.tga");
	remove("output/temporary.tga");
	imageData top = readFile("input/text.tga");
	screen(top, bottom, "output/part3.tga");
	compareTest("output/part3.tga", "examples/EXAMPLE_part3.tga");
}

void test4()
{
	cout << "Test #4" << endl;
	imageData one = readFile("input/layer2.tga");
	imageData two = readFile("input/circles.tga");
	multiply(one, two, "output/temporary.tga");
	imageData bottom = readFile("output/temporary.tga");
	remove("output/temporary.tga");
	imageData top = readFile("input/pattern2.tga");
	subtract(top, bottom, "output/part4.tga");
	compareTest("output/part4.tga", "examples/EXAMPLE_part4.tga");
}

void test5()
{
	cout << "Test #5" << endl;
	imageData top = readFile("input/layer1.tga");
	imageData bottom = readFile("input/pattern1.tga");
	overlay(top, bottom, "output/part5.tga");
	compareTest("output/part5.tga", "examples/EXAMPLE_part5.tga");
}

void test6()
{
	cout << "Test #6" << endl;
	imageData one = readFile("input/car.tga");
	add(one, "output/part6.tga", 0, 200, 0);
	compareTest("output/part6.tga", "examples/EXAMPLE_part6.tga");
}

void test7()
{
	cout << "Test #7" << endl;
	imageData one = readFile("input/car.tga");
	scale(one, "output/part7.tga", 0, 1, 4);
	compareTest("output/part7.tga", "examples/EXAMPLE_part7.tga");
}

void test8()
{
	imageData one = readFile("input/car.tga");
	seperate(one, "output/part8_b.tga", "output/part8_g.tga", "output/part8_r.tga");
	cout << "Test #8 a" << endl;
	compareTest("output/part8_b.tga", "examples/EXAMPLE_part8_b.tga");
	cout << "Test #8 b" << endl;
	compareTest("output/part8_g.tga", "examples/EXAMPLE_part8_g.tga");
	cout << "Test #8 c" << endl;
	compareTest("output/part8_r.tga", "examples/EXAMPLE_part8_r.tga");
}

void test9()
{
	cout << "Test #9" << endl;
	imageData blue = readFile("input/layer_blue.tga");
	imageData green = readFile("input/layer_green.tga");
	imageData red = readFile("input/layer_red.tga");
	combine(blue, green, red, "output/part9.tga");
	compareTest("output/part9.tga", "examples/EXAMPLE_part9.tga");
}

void test10()
{
	imageData one = readFile("input/text2.tga");
	flip(one, "output/part10.tga");
	cout << "Test #10" << endl;
	compareTest("output/part10.tga", "examples/EXAMPLE_part10.tga");
}

void testExtraCredit()
{
	imageData bottomLeft = readFile("input/text.tga");
	imageData bottomRight = readFile("input/pattern1.tga");
	imageData topLeft = readFile("input/car.tga");
	imageData topRight = readFile("input/circles.tga");
	four(bottomLeft, bottomRight, topLeft, topRight, "output/extraCredit.tga");
	cout << "Test extra credit" << endl;
	compareTest("output/extraCredit.tga", "examples/EXAMPLE_extraCredit.tga");
}

int main()
{
	test1();
	test2();
	test3();
	test4();
	test5();
	test6();
	test7();
	test8();
	test9();
	test10();
	testExtraCredit();
	return 0;
}


