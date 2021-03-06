/* ***********************************************************************
 *
 * filename:            $Source: /cvsroot/sql2diagram/sql2diagram/src/databaseDIA.cpp,v $
 * revision:            $Revision: 1.6 $
 * last changes:        $Date: 2013/06/17 18:51:56 $
 * Author:              Timotheus Pokorra (timotheus at pokorra.de)
 * Feel free to use the code in this file in your own projects...
 *
 ********************************************************************** */
#include "dia.h"
#include <stdio.h>

void DataBaseDIA::prepareDisplay(string module, string tableList, const string& strColorTableIgnoreReferencedTables, bool repeatedRun)
{
	vector<Table>::iterator it;
	m_module = module;

	tables.clear();
	/* TODO : Maybe add regular expressions ( using "man regexp") */
	for (it = allTables.begin(); it != allTables.end(); it++) {
		if ( cmpModule(it->getModule(), m_module)
      || inTableList( *it, tableList)
      || ( module == NONDISPLAYED && !it->isDisplayedAlready())) {
			tables.push_back(*it);
		}
	}

	Table::resetColumn(tables.size());
	for (it = tables.begin(); it != tables.end(); it++) {
		Table& tab = *it;
		((TableDIA*)&tab)->prepareDisplay(*this, m_module, repeatedRun, tableList, strColorTableIgnoreReferencedTables);
	}

	// merge Positions of associations
	vector<PosAssociation>::iterator at;
	for ( at = posAssociations.begin();
			at != posAssociations.end();
			at++) {
		Table& src = getFromId(at->connections[0]);
		if ( !src.getName().empty()) {
			src.setPositionConstraint( at->connectionPoints[0],
												getFromId(at->connections[1]),
												at->obj_pos, at->obj_bb, at->orth_orient, at->orth_points);
		}
	}

	sort( tables.begin(), tables.end());
}

void DataBaseDIA::outDiaPngCrop(FILE* file, string diagramname)
// prepare batch file that calls ImageMagick Convert for tiling the big image
// that helps to print the images
{
	float left, top, bottom, right;
	vector<Table>::iterator it;
	right = left = tables.begin()->getPosition().x;
	bottom = top = tables.begin()->getPosition().y;
	for ( it = tables.begin(); it != tables.end(); it++) {
		if (it->isVisible()) {
			if (left > it->getPosition().x) {
				left = it->getPosition().x;
			}
			if (right < it->getPosition().x+it->getWidth()) {
				right = it->getPosition().x+it->getWidth();
			}
			if (top > it->getPosition().y) {
				top = it->getPosition().y;
			}
			if (bottom < it->getPosition().y+it->getHeight()) {
				bottom = it->getPosition().y+it->getHeight();
			}
		}
	}
	int width = int(right-left+1)*20;
	int height = int(bottom-top+1)*20;

	if ( file) {
		int count = 0;
		int tilewidth = 1024;
		int tileheight = 768;
		int x, y;
        char s[20];
		for ( y = 0; y < height; y += tileheight) {
			for (x = 0; x < width; x += tilewidth, count++) {
                sprintf(s, "%d", count);
				fprintf( file, "%%1/convert -crop %dx%d+%d+%d %s.png %s.png\n",
							tilewidth, tileheight, x, y,
							diagramname.c_str(), (diagramname+s).c_str());
			}
		}
	}
}

void DataBaseDIA::outDia(FILE* file, bool repeatedRun, const string& strLocTableList, const string& strColorTableIgnoreReferencedTables)
{
	vector<Table>::iterator it;
	Table::resetColumn(-1);
	if ( file) {
		fprintf( file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf( file, "<dia:diagram xmlns:dia=\"http://www.lysator.liu.se/~alla/dia/\"><dia:diagramdata>\n");
		fprintf( file, "<dia:attribute name=\"invisible\"><dia:color val=\"#ffffff\"/></dia:attribute><dia:attribute name=\"paper\"><dia:composite type=\"paper\"><dia:attribute name=\"name\"><dia:string>#A3#</dia:string></dia:attribute><dia:attribute name=\"tmargin\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"bmargin\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"lmargin\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"rmargin\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"is_portrait\"><dia:boolean val=\"true\"/></dia:attribute><dia:attribute name=\"scaling\"><dia:real val=\"1.00\"/></dia:attribute><dia:attribute name=\"fitto\"><dia:boolean val=\"false\"/></dia:attribute></dia:composite></dia:attribute><dia:attribute name=\"grid\"><dia:composite type=\"grid\"><dia:attribute name=\"width_x\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"width_y\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"visible_x\"><dia:int val=\"1\"/></dia:attribute><dia:attribute name=\"visible_y\"><dia:int val=\"1\"/></dia:attribute></dia:composite></dia:attribute><dia:attribute name=\"guides\"><dia:composite type=\"guides\"><dia:attribute name=\"hguides\"/><dia:attribute name=\"vguides\"/></dia:composite></dia:attribute>\n");
		fprintf( file, "<dia:attribute name=\"background\"><dia:color val=\"#ffffff\"/></dia:attribute><dia:attribute name=\"paper\"><dia:composite type=\"paper\"><dia:attribute name=\"name\"><dia:string>#A3#</dia:string></dia:attribute><dia:attribute name=\"tmargin\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"bmargin\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"lmargin\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"rmargin\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"is_portrait\"><dia:boolean val=\"true\"/></dia:attribute><dia:attribute name=\"scaling\"><dia:real val=\"1.00\"/></dia:attribute><dia:attribute name=\"fitto\"><dia:boolean val=\"false\"/></dia:attribute></dia:composite></dia:attribute><dia:attribute name=\"grid\"><dia:composite type=\"grid\"><dia:attribute name=\"width_x\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"width_y\"><dia:real val=\"1\"/></dia:attribute><dia:attribute name=\"visible_x\"><dia:int val=\"1\"/></dia:attribute><dia:attribute name=\"visible_y\"><dia:int val=\"1\"/></dia:attribute></dia:composite></dia:attribute><dia:attribute name=\"guides\"><dia:composite type=\"guides\"><dia:attribute name=\"hguides\"/><dia:attribute name=\"vguides\"/></dia:composite></dia:attribute>\n");
		fprintf( file, "</dia:diagramdata>\n");
		fprintf( file, "<dia:layer name=\"Background\" visible=\"true\">\n");
	}

	if ( !repeatedRun) {
		sort( tables.begin(), tables.end());
	}
	for ( it = tables.begin();
			it != tables.end();
			it++) {
		if (it->isVisible() && repeatedRun) {
			Table& tab = *it;
			((TableDIA*)&tab)->outDia(file, m_module, *this, repeatedRun, strLocTableList, strColorTableIgnoreReferencedTables);
			it->SetDisplayedAlready(true, m_module);
			getAllTable(it->getName()).SetDisplayedAlready(true, m_module);
		}
	}

	// constraints
	if ( repeatedRun) {
		for ( it = tables.begin();
				it != tables.end();
				it++) {
			Table& tab = *it;
			((TableDIA*)&tab)->outDiaConstraints(file, *this, strLocTableList);
		}
	}

	if ( file) {
		fprintf(file, "</dia:layer>\n");
		if (repeatedRun) {
			fprintf(file, "<dia:layer name=\"invisible\" visible=\"false\">\n");
		} else {
			fprintf(file, "<dia:layer name=\"invisible\" visible=\"true\">\n");
		}
	}

	for ( it = tables.begin();
			it != tables.end();
			it++) {
		if ( !it->isVisible()
		||   !repeatedRun) {
			Table& tab = *it;
			((TableDIA*)&tab)->outDia(file, m_module, *this, repeatedRun, strLocTableList, strColorTableIgnoreReferencedTables, true);
		}
	}

	if ( file) {
		fprintf(file, "</dia:layer>\n");
		fprintf(file, "</dia:diagram>\n");
	}
}
