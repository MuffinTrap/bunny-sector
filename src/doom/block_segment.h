#pragma once

class BlockSegment
{
public:
	int start;
	int end;
	int nextIndex;
	int prevIndex;
	int myIndex;
	bool isPortal;

	BlockSegment();
	BlockSegment(int left, int right, int prev, int next, int index, bool portal);

	bool OverlapsWith(BlockSegment& other);
	void MergeInto(BlockSegment& other);
	void Drop();
};


void ResetBlockSegments();

bool IsScreenBlocked();

bool IsSegmentBlocked(int startx, int endx);

void PushBlockSegment(int startx, int endx, bool isPortal);

void InsertNewSegment(BlockSegment& drawn);
