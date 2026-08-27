
bool ChildIsNode(ChildId childId)
{
	return (childId & 0x80000000 ) == 0;
}
bool ChildIsSector(ChildId childId)
{
	return (childId & 0x80000000 ) > 0;
}

bool IsOnBackSide(DoomNode@ node, float x, float y)
{
	float dx = x - node.x;
	float dy = y - node.y;
	return dx * node.dy - dy * node.dx <= 0;
}

int drawOrder = 0;

void DrawSubSector(DoomMap@ map, DoomSubSector@ sub)
{
	color32 pcolor = Debug_DarkGray;
	if (drawOrder == 0)
	{
		pcolor = Debug_Yellow;
	}
	for (uint i = 0; i < sub.segmentAmount; i++)
	{
		uint next = (i+1)%sub.segmentAmount;
		DoomSegment@ seg = sub.segments[i];
		DoomSegment@ partner = sub.segments[next];

		DoomVertex@ v1 = map.vertices[seg.v1];
		DoomVertex@ v2 = map.vertices[partner.v1];
		glBegin(GL_LINES);
		mgdl_glColor32(pcolor);
		glVertex2f(v1.x, v1.y);
		glVertex2f(v2.x, v2.y);
		glEnd();
	}
	DoomSegment@ seg = sub.segments[0];
	DoomVertex@ v1 = map.vertices[seg.v1];
	mgdl_DrawTextInt("p", drawOrder, v1.x, v1.y - 16, 16, pcolor);
	drawOrder += 1;
}

void DrawNodeChild(DoomMap@ map, ChildId id)
{
	if (ChildIsNode(id))
	{
		DrawNode(map, map.GetChildNode(id));
	}
	else
	{
		DoomSubSector@ sub =  map.GetChildSubSector(id);
		DrawSubSector(map, sub);
	}
}

void DrawNode(DoomMap@ map, DoomNode@ node)
{
	DoomThing@ player = map.things[0];

	glBegin(GL_LINE_LOOP);
		glColor3f(0.0f, 0.0f, 0.0f);
		glVertex2i(node.left0, node.top0);
		glVertex2i(node.left0, node.bottom0);
		glVertex2i(node.right0, node.bottom0);
		glVertex2i(node.right0, node.top0);
	glEnd();
	// This determines which branch is done first so that
	// eventually the players subsector is drawn first
	bool isOnBack = IsOnBackSide(node, player.x, player.y);

	if (isOnBack)
	{
		DrawNodeChild(map, node.child1);
		DrawNodeChild(map, node.child0);
	}
	else
	{
		DrawNodeChild(map, node.child0);
		DrawNodeChild(map, node.child1);
	}

	/*
	glBegin(GL_LINES);
		glColor3f(0.0f, 1.0f, 1.0f);
		glVertex2i(node.x, node.y);
		glVertex2i(node.x + node.dx * 100, node.y + node.dy * 100);
	glEnd();
	*/
}

void RenderDoomMap(DoomMap@ map)
{
	drawOrder = 0;

	glClearColor(0.5f, 0.02f, 0.1f, 1.0f);
	Init2D_YDown();

	float screen_half_width = SCREEN_WIDTH / 2.0f;
	float screen_half_height = SCREEN_HEIGHT / 2.0f;

glPushMatrix();
	u32 va = map.vertexAmount;
	int vv = va;
	mgdl_DrawTextInt("Vertices x: ", vv, 8, 8, 8, Debug_Yellow);

	glTranslatef(screen_half_width, screen_half_height, 0);
	glScalef(0.5f, 0.5f, 1.0f);

	// Draw Nodes and bounding boxes
	DoomNode@ root = map.GetRootNode();
	DrawNode(map, root);


	// Draw vertices
	glBegin(GL_POINTS);
	glColor3f(1.0f, 1.0f, 1.0f);
	for (uint i = 0; i < map.vertexAmount; i++)
	{
		DoomVertex@ v = map.vertices[i];
		DrawCross(Vector2New(v.x, v.y), Debug_White);
	}
	glEnd();

	DoomThing@ player = map.things[0];
	DrawCross(Vector2New(player.x, player.y), Debug_Blue);

glPopMatrix();

}

