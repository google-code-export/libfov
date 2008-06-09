/*
 * Copyright (C) 2006, Greg McIntyre
 * All rights reserved. See the file named COPYING in the distribution
 * for more details.
 */

#include <sstream>
#include <iostream>
#include <list>
#include <map>
#include <vector>
#include <fov/fov.h>

class Map;

// -------------------------------------------------

class CountMap {
private:
	std::vector<unsigned> counts;
    
public:
    unsigned w;
    unsigned h;
	CountMap(unsigned w, unsigned h, const char **counts=NULL);
    bool operator==(const CountMap& b) const;
	bool isOnMap(unsigned x, unsigned y) const;
	unsigned value(unsigned x, unsigned y) const;
	void increment(unsigned x, unsigned y);
};

CountMap::CountMap(unsigned w, unsigned h, const char **counts) {
    this->w = w;
    this->h = h;
    this->counts.resize(w*h);
    for (unsigned j = 0; j < h; ++j) {
        for (unsigned i = 0; i < w; ++i) {
            if (counts != NULL) {
                this->counts[j*w+i] = counts[h - 1 - j][i] - '0';
            } else {
                this->counts[j*w+i] = 0;
            }
        }
    }
}

bool CountMap::isOnMap(unsigned x, unsigned y) const {
	return x < w && y < h;
}

unsigned CountMap::value(unsigned x, unsigned y) const
{
	if (isOnMap(x, y)) {
        return counts[y*w+x];
    } else {
        return 0;
    }
}

void CountMap::increment(unsigned x, unsigned y)
{
	if (isOnMap(x, y)) {
        counts[y*w+x] += 1;
    }
}

bool CountMap::operator==(const CountMap& b) const
{
    return w == b.w && h == b.h && counts == b.counts;
}

std::ostream& operator<<(std::ostream& out, const CountMap& map)
{
    for (int j = (int)map.h - 1; j >= 0; --j) {
        for (int i = 0; i < (int)map.w; ++i) {
            char str[2] = { ('0' + map.value(i, j)), '\0' };
            out << str;
        }
        out << std::endl;
    }
    return out;
}

// -------------------------------------------------

class Cell {
	friend class Map;

public:
	unsigned char tile;
    bool seen;

    Cell(void);
    void apply(void);
    bool isOpaque(void) const;
};

Cell::Cell(void) {
    tile = '.';
    seen = false;
}

void Cell::apply(void) {
    seen = true;
}

bool Cell::isOpaque(void) const {
    return tile == '#';
}

// -------------------------------------------------

class Map {

public:

	Cell *cells;
    unsigned w;
    unsigned h;

    CountMap opaqueCountMap;
    CountMap applyCountMap;

	Map(unsigned w, unsigned h, const char **raster);
    Map(const Map& map);
	virtual ~Map(void);
	void apply(unsigned x, unsigned y);
	bool isOnMap(unsigned x, unsigned y);
	bool isOpaque(unsigned x, unsigned y);
};

Map::Map(unsigned w, unsigned h, const char **raster):
    opaqueCountMap(w, h), applyCountMap(w, h)
{
    this->w = w;
    this->h = h;
    this->cells = new Cell[w*h];
    for (unsigned i = 0; i < w; ++i) {
        for (unsigned j = 0; j < h; ++j) {
            Cell& c = this->cells[j*w+i];
            c.tile = raster[h - 1 - j][i];
        }
    }
}

Map::Map(const Map& map):
    opaqueCountMap(map.opaqueCountMap), applyCountMap(map.applyCountMap)
{
    this->w = map.w;
    this->h = map.h;
    this->cells = new Cell[w*h];
    memcpy(this->cells, map.cells, w*h*sizeof(Cell));
}

Map::~Map() {
    if (this->cells) {
        delete[] this->cells;
        this->cells = NULL;
    }
}

bool Map::isOnMap(unsigned x, unsigned y) {
	return x < w && y < h;
}

bool Map::isOpaque(unsigned x, unsigned y) {
    if (isOnMap(x, y)) {
        opaqueCountMap.increment(x, y);
        return cells[y*w+x].isOpaque();
    } else {
        return true;
    }
}

void Map::apply(unsigned x, unsigned y)
{
	if (isOnMap(x, y)) {
        cells[y*w+x].apply();
        applyCountMap.increment(x, y);
    }
}

std::ostream& operator<<(std::ostream& out, const Map& map)
{
    for (int j = (int)map.h - 1; j >= 0; --j) {
        for (int i = 0; i < (int)map.w; ++i) {
            char str[2] = { map.cells[j*map.w+i].tile, '\0' };
            out << str;
        }
        out << std::endl;
    }
    return out;
}

// -------------------------------------------------

static void apply(void *map, int x, int y, int dx, int dy, void *src) {
    Map& m = *((Map *)map);
    m.apply(x, y);
}

static bool opaque(void *map, int x, int y) {
    Map& m = *((Map *)map);
    return m.isOpaque(x, y);
}

// -------------------------------------------------

typedef std::pair<CountMap, CountMap> ResultRasterPair;
typedef std::pair<Map, ResultRasterPair> InputAndExpected;
typedef std::list<InputAndExpected> ExpectedResults;

class TestFOV {
    std::ostringstream& errors;
    char result;
    unsigned failures;
public:

    TestFOV(std::ostringstream& errors);

    unsigned numberOfFailures();

    void testBasics();
    void testCircle();
    void testOctagon();
    void testWallFace();
    void testBeam();
    void testGrow();
    void testSeeBehindOrthogonal();
private:
    void doTestCircle(fov_settings_type* settings,
                 int px, int py, unsigned radius,
                 ExpectedResults& expected);
    void doTestBeam(fov_settings_type* settings,
                    int px, int py, unsigned radius,
                    fov_direction_type direction, float angle,
                    ExpectedResults expected);
    void assertCountMapsEqual(const Map& map,
                              const CountMap& expected,
                              const CountMap& found,
                              const char *name);
};

TestFOV::TestFOV(std::ostringstream& errors):
    errors(errors), result('.'), failures(0) {
}

// -------------------------------------------------

void TestFOV::assertCountMapsEqual(const Map& map,
                                   const CountMap& expected,
                                   const CountMap& found,
                                   const char *name) {
    if (!(expected == found)) {
        errors << "--" << std::endl;
        errors << "Error in map:" << std::endl << map;
        errors << "Expected " << name << " map was:" << std::endl;
        errors << expected;
        errors << "Evaluated " << name << " map was:" << std::endl;
        errors << found << "--" << std::endl;
        result = 'F';
        failures++;
    }
}

void TestFOV::doTestCircle(fov_settings_type* settings,
                      int px, int py, unsigned radius,
                      ExpectedResults& expected) {
    for (ExpectedResults::iterator i = expected.begin(); i != expected.end(); ++i) {
        result = '.';
        Map& map = i->first;
        fov_circle(settings, &map, NULL, px, py, radius);
        assertCountMapsEqual(map, i->second.first, map.applyCountMap, "apply");
        assertCountMapsEqual(map, i->second.second, map.opaqueCountMap, "opaque");
        std::cout << result;
    }
}

void TestFOV::doTestBeam(fov_settings_type* settings,
                         int px, int py, unsigned radius,
                         fov_direction_type direction, float angle,
                         ExpectedResults expected) {
    std::ostringstream errors;
    for (ExpectedResults::iterator i = expected.begin(); i != expected.end(); ++i) {
        result = '.';
        Map& map = i->first;
        fov_beam(settings, &map, NULL, px, py, radius, direction, angle);
        assertCountMapsEqual(map, i->second.first, map.applyCountMap, "apply");
        assertCountMapsEqual(map, i->second.second, map.opaqueCountMap, "opaque");
        std::cout << result;
    }
}

unsigned TestFOV::numberOfFailures() {
    return this->failures;
}

void TestFOV::testBasics() {
    static const unsigned int radius = 3;
    static const unsigned int w = 10;
    static const unsigned int h = 10;
    static const int px = 4;
    static const int py = 4;

    ExpectedResults expected;

    fov_settings_type settings;

    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque);
    fov_settings_set_apply_lighting_function(&settings, apply);
    fov_settings_set_shape(&settings, FOV_SHAPE_SQUARE);

    static const char *t1r[] = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "....@.....",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    static const char *t1a[] = {
        "0000000000",
        "0000000000",
        "0111111100",
        "0111111100",
        "0111111100",
        "0111011100",
        "0111111100",
        "0111111100",
        "0111111100",
        "0000000000",
    };
    static const char *t1o[] = {
        "0000000000",
        "0000000000",
        "0111211100",
        "0111211100",
        "0111211100",
        "0222022200",
        "0111211100",
        "0111211100",
        "0111211100",
        "0000000000",
    };
    expected.push_back(InputAndExpected(Map(w, h, t1r), ResultRasterPair(CountMap(w, h, t1a), CountMap(w, h, t1o))));

    static const char *t2r[] = {
        "..........",
        "..........",
        "..........",
        "..........",
        "...###....",
        "...#@#....",
        "...###....",
        "..........",
        "..........",
        "..........",
    };
    static const char *t2a[] = {
        "0000000000",
        "0000000000",
        "0000000000",
        "0000000000",
        "0001110000",
        "0001010000",
        "0001110000",
        "0000000000",
        "0000000000",
        "0000000000",
    };
    static const char *t2o[] = {
        "0000000000",
        "0000000000",
        "0000000000",
        "0000000000",
        "0001210000",
        "0002020000",
        "0001210000",
        "0000000000",
        "0000000000",
        "0000000000",
    };
    expected.push_back(InputAndExpected(Map(w, h, t2r), ResultRasterPair(CountMap(w, h, t2a), CountMap(w, h, t2o))));

    static const char *t3r[] = {
        "..........",
        "..........",
        "..........",
        ".....#####",
        "##########",
        "....@.....",
        "..........",
        "..........",
        "..........",
        "..........",
    };
    static const char *t3a[] = {
        "0000000000",
        "0000000000",
        "0000000000",
        "0000000000",
        "0111111100",
        "0111011100",
        "0111111100",
        "0111111100",
        "0111111100",
        "0000000000",
    };
    static const char *t3o[] = {
        "0000000000",
        "0000000000",
        "0000000000",
        "0000000000",
        "0111211100",
        "0222022200",
        "0111211100",
        "0111211100",
        "0111211100",
        "0000000000",
    };
    expected.push_back(InputAndExpected(Map(w, h, t3r), ResultRasterPair(CountMap(w, h, t3a), CountMap(w, h, t3o))));

    static const char *t4r[] = {
        "..........",
        "..........",
        "..........",
        "..........",
        "..........",
        "....@####.",
        "......###.",
        "..........",
        "..........",
        "..........",
    };
    static const char *t4a[] = {
        "0000000000",
        "0000000000",
        "0111111100",
        "0111111000",
        "0111110000",
        "0111010000",
        "0111110000",
        "0111111000",
        "0111111100",
        "0000000000",
    };
    static const char *t4o[] = {
        "0000000000",
        "0000000000",
        "0111211100",
        "0111211000",
        "0111210000",
        "0222020000",
        "0111210000",
        "0111211000",
        "0111211100",
        "0000000000",
    };
    expected.push_back(InputAndExpected(Map(w, h, t4r), ResultRasterPair(CountMap(w, h, t4a), CountMap(w, h, t4o))));

    doTestCircle(&settings, px, py, radius, expected);
    fov_settings_free(&settings);
}

void TestFOV::testCircle(void) {
    static const unsigned int radius = 6;
    static const unsigned int w = 15;
    static const unsigned int h = 15;
    static const int px = 7;
    static const int py = 7;

    ExpectedResults expected;

    fov_settings_type settings;

    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque);
    fov_settings_set_apply_lighting_function(&settings, apply);

    static const char *t1r[] = {
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        ".......@.......",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
    };
    static const char *t1a[] = {
        "000000000000000",
        "000000000000000",
        "000011111110000",
        "000111111111000",
        "001111111111100",
        "001111111111100",
        "001111111111100",
        "001111101111100",
        "001111111111100",
        "001111111111100",
        "001111111111100",
        "000111111111000",
        "000011111110000",
        "000000000000000",
        "000000000000000",
    };
    static const char *t1o[] = {
        "000000000000000",
        "000000000000000",
        "000011121110000",
        "000111121111000",
        "001111121111100",
        "001111121111100",
        "001111121111100",
        "002222202222200",
        "001111121111100",
        "001111121111100",
        "001111121111100",
        "000111121111000",
        "000011121110000",
        "000000000000000",
        "000000000000000",
    };
    expected.push_back(InputAndExpected(Map(w, h, t1r), ResultRasterPair(CountMap(w, h, t1a), CountMap(w, h, t1o))));

    doTestCircle(&settings, px, py, radius, expected);
    fov_settings_free(&settings);
}

void TestFOV::testOctagon(void) {
    static const unsigned int radius = 6;
    static const unsigned int w = 15;
    static const unsigned int h = 15;
    static const int px = 7;
    static const int py = 7;

    ExpectedResults expected;

    fov_settings_type settings;

    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque);
    fov_settings_set_apply_lighting_function(&settings, apply);
    fov_settings_set_shape(&settings, FOV_SHAPE_OCTAGON);

    static const char *t1r[] = {
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        ".......@.......",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
    };
    static const char *t1a[] = {
        "000000000000000",
        "000000000000000",
        "000001111100000",
        "000111111111000",
        "000111111111000",
        "001111111111100",
        "001111111111100",
        "001111101111100",
        "001111111111100",
        "001111111111100",
        "000111111111000",
        "000111111111000",
        "000001111100000",
        "000000000000000",
        "000000000000000",
    };
    static const char *t1o[] = {
        "000000000000000",
        "000000000000000",
        "000001121100000",
        "000111121111000",
        "000111121111000",
        "001111121111100",
        "001111121111100",
        "002222202222200",
        "001111121111100",
        "001111121111100",
        "000111121111000",
        "000111121111000",
        "000001121100000",
        "000000000000000",
        "000000000000000",
    };
    expected.push_back(InputAndExpected(Map(w, h, t1r), ResultRasterPair(CountMap(w, h, t1a), CountMap(w, h, t1o))));

    doTestCircle(&settings, px, py, radius, expected);
    fov_settings_free(&settings);
}

/**
 * This tests that wall faces are lit even when the source is up
 * against the wall creating very shallow beam angles hitting the
 * wall.
 */
void TestFOV::testWallFace(void) {
    static const unsigned int radius = 40;
    static const unsigned int w = 30;
    static const unsigned int h = 4;
    static const int px = 0;
    static const int py = 1;

    ExpectedResults expected;

    fov_settings_type settings;

    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque);
    fov_settings_set_apply_lighting_function(&settings, apply);

    static const char *t1r[] = {
        "..............................",
        "##############################",
        "@.............................",
        "..............................",
    };
    static const char *t1a[] = {
        "000000000000000000000000000000",
        "111111111111111111111111111111",
        "011111111111111111111111111111",
        "111111111111111111111111111111",
    };
    static const char *t1o[] = {
        "000000000000000000000000000000",
        "211111111111111111111111111111",
        "022222222222222222222222222222",
        "211111111111111111111111111111",
    };
    expected.push_back(InputAndExpected(Map(w, h, t1r), ResultRasterPair(CountMap(w, h, t1a), CountMap(w, h, t1o))));

    doTestCircle(&settings, px, py, radius, expected);
    fov_settings_free(&settings);
}

void TestFOV::testBeam(void) {
    static const unsigned int radius = 20;
    static const unsigned int w = 15;
    static const unsigned int h = 15;
    static const int px = 0;
    static const int py = 7;

    ExpectedResults expected;

    fov_settings_type settings;

    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque);
    fov_settings_set_apply_lighting_function(&settings, apply);

    static const char *t1r[] = {
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "@..............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
    };
    static const char *t1a[] = {
        "000000000000011",
        "000000000001111",
        "000000000111111",
        "000000011111111",
        "000001111111111",
        "000111111111111",
        "011111111111111",
        "011111111111111",
        "011111111111111",
        "000111111111111",
        "000001111111111",
        "000000011111111",
        "000000000111111",
        "000000000001111",
        "000000000000011",
    };
    static const char *t1o[] = {
        "000000000000011",
        "000000000001111",
        "000000000111111",
        "000000011111111",
        "000001111111111",
        "000111111111111",
        "011111111111111",
        "022222222222222",
        "011111111111111",
        "000111111111111",
        "000001111111111",
        "000000011111111",
        "000000000111111",
        "000000000001111",
        "000000000000011",
    };
    expected.push_back(InputAndExpected(Map(w, h, t1r), ResultRasterPair(CountMap(w, h, t1a), CountMap(w, h, t1o))));

    doTestBeam(&settings, px, py, radius, FOV_EAST, 45.0f, expected);
    fov_settings_free(&settings);
}

void TestFOV::testGrow(void) {
    static const unsigned int w = 15;
    static const unsigned int h = 15;
    static const int px = 0;
    static const int py = 7;

    ExpectedResults expected;

    fov_settings_type settings;
    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque);
    fov_settings_set_apply_lighting_function(&settings, apply);

    static const char *t1r[] = {
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "@..............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
    };
    static const char *t1a[] = {
        "000000000000011",
        "000000000001111",
        "000000000111111",
        "000000011111111",
        "000001111111111",
        "000111111111111",
        "011111111111111",
        "011111111111111",
        "011111111111111",
        "000111111111111",
        "000001111111111",
        "000000011111111",
        "000000000111111",
        "000000000001111",
        "000000000000011",
    };
    static const char *t1o[] = {
        "000000000000011",
        "000000000001111",
        "000000000111111",
        "000000011111111",
        "000001111111111",
        "000111111111111",
        "011111111111111",
        "022222222222222",
        "011111111111111",
        "000111111111111",
        "000001111111111",
        "000000011111111",
        "000000000111111",
        "000000000001111",
        "000000000000011",
    };
    expected.push_back(InputAndExpected(Map(w, h, t1r), ResultRasterPair(CountMap(w, h, t1a), CountMap(w, h, t1o))));

    doTestBeam(&settings, px, py, 20, FOV_EAST, 45.0f, expected);
    /* Again, to re-allocate the heights array. */
    doTestBeam(&settings, px, py, 20000, FOV_EAST, 45.0f, expected);
    fov_settings_free(&settings);
}

void TestFOV::testSeeBehindOrthogonal(void) {
    static const unsigned int radius = 20;
    static const unsigned int w = 15;
    static const unsigned int h = 15;
    static const int px = 0;
    static const int py = 7;

    ExpectedResults expected;

    fov_settings_type settings;
    fov_settings_init(&settings);
    fov_settings_set_opacity_test_function(&settings, opaque);
    fov_settings_set_apply_lighting_function(&settings, apply);

    static const char *t1r[] = {
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "@......#.......",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
        "...............",
    };
    static const char *t1a[] = {
        "000000000000011",
        "000000000001111",
        "000000000111111",
        "000000011111111",
        "000001111111111",
        "000111111111111",
        "011111111111111",
        "011111110000000",
        "011111111111111",
        "000111111111111",
        "000001111111111",
        "000000011111111",
        "000000000111111",
        "000000000001111",
        "000000000000011",
    };
    static const char *t1o[] = {
        "000000000000011",
        "000000000001111",
        "000000000111111",
        "000000011111111",
        "000001111111111",
        "000111111111111",
        "011111111111111",
        "022222220000000",
        "011111111111111",
        "000111111111111",
        "000001111111111",
        "000000011111111",
        "000000000111111",
        "000000000001111",
        "000000000000011",
    };
    expected.push_back(InputAndExpected(Map(w, h, t1r), ResultRasterPair(CountMap(w, h, t1a), CountMap(w, h, t1o))));

    doTestBeam(&settings, px, py, radius, FOV_EAST, 45.0f, expected);
    fov_settings_free(&settings);
}

/**
 * Program entry point.
 */
int main (int argc, char *argv[]) {
    std::cout << "--- Testing libfov ----------------------" << std::endl;
    std::ostringstream errors;
    TestFOV t(errors);

    t.testCircle();
    t.testBasics();
    t.testOctagon();
    t.testWallFace();
    t.testBeam();
    t.testGrow();
    t.testSeeBehindOrthogonal();

    std::cout << std::endl;
    std::cout << errors.str() << std::endl;
    return t.numberOfFailures() > 0 ? 1 : 0;
}
