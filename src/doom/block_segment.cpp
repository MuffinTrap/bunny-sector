#include "block_segment.h"
#include "../bunny-sector-math.h"
#include "doom_render_util.h"
#include <mgdl.h>

const int BLOCK_SEGMENT_AMOUNT = 64;
static BlockSegment blockSegments[BLOCK_SEGMENT_AMOUNT];
static int lastBlockSegment = 0;
static int MAX_16 = 0x7FFF;


BlockSegment::BlockSegment()
{
	start = 0;
	end = 0;
	nextIndex = -1;
	prevIndex = -1;
	myIndex = -1;
	isPortal = false;
}

BlockSegment::BlockSegment(int left, int right, int prev, int next, int index, bool portal)
{
	start = left;
	end = right;
	prevIndex = prev;
	nextIndex = next;
	myIndex = index;
	isPortal = portal;
}

bool BlockSegment::OverlapsWith(BlockSegment& other)
{

	// Overlap:  Determine whether the two number ranges overlap.
	// #define Overlap(a0,a1,b0,b1) (map_min(a0,a1) <= map_max(b0,b1) && map_min(b0,b1) <= map_max(a0,a1))

	if (MinI(start, end) <= MaxI(other.start, other.end) && MinI(other.start, other.end) <= MaxI(start, end))
	{
		return true;
	}
	return false;

}

void BlockSegment::MergeInto(BlockSegment& other)
{
	if (start >= other.start && end <= other.end)
	{
		// This is inside the other!
	}
	if (start < other.start)
	{
		other.start = start;
	}
	if (end > other.end)
	{
		other.end = end;
	}
}

	void BlockSegment::Drop()
	{
		if (prevIndex >= 0)
		{
			BlockSegment& prevSegment = blockSegments[prevIndex];
			prevSegment.nextIndex = nextIndex;
		}
		if (nextIndex >= 0)
		{
			BlockSegment& nextSegment = blockSegments[nextIndex];
			nextSegment.prevIndex = prevIndex;
		}
	}


void ResetBlockSegments()
{
	blockSegments[0] = BlockSegment(-MAX_16, -SCREEN_WIDTH/2, -1, 1, 0, false);
	blockSegments[1] = BlockSegment(SCREEN_WIDTH/2, MAX_16, 0, -1, 1, false);
	lastBlockSegment = 2;
}

bool IsScreenBlocked()
{
	if (blockSegments[0].start <= -MAX_16 + 1 && blockSegments[0].end >= MAX_16-1)
	{
		//printf(">>>>>>>>>>  Screen blocked\n");
		return true;
	}
	return false;
}

bool IsSegmentBlocked(int startx, int endx)
{
	int nextIndex = 0;
	while(nextIndex >= 0)
	{
		BlockSegment& seg = blockSegments[nextIndex];
		if (startx >= seg.start && endx <= seg.end)
		{
			return true;
		}
		nextIndex = seg.nextIndex;
	}
	return false;
}

void PushBlockSegment(int startx, int endx, bool isPortal)
{
	//printf("Push segment %d -> %d\n", startx, endx);
	BlockSegment& drawn = blockSegments[lastBlockSegment];
	drawn.start = startx;
	drawn.end = endx;
	drawn.isPortal = isPortal;
	InsertNewSegment(drawn);
}

void InsertNewSegment(BlockSegment& drawn)
{
	bool merged = false;

	// Remember what was the previous target
	int previousTargetIndex = -1;

	// Start with targeting the first
	int targetIndex = 0;
	while(targetIndex >= 0 && targetIndex < BLOCK_SEGMENT_AMOUNT)
	{
		BlockSegment& target = blockSegments[targetIndex];

		// 1. Try merge
		if (drawn.OverlapsWith(target))
		{
			drawn.MergeInto(target);
			merged = true;
			break;
		} // 2. Insert?
		else if (drawn.end < target.start)
		{
			// Get my index in list
			drawn.myIndex = lastBlockSegment;
			// Set the previous target as my previous segment
			drawn.prevIndex = previousTargetIndex;

			// If the previous one was not -1
			if (previousTargetIndex >= 0)
			{
				BlockSegment& previous = blockSegments[previousTargetIndex];
				// Make me the next one of previous target
				previous.nextIndex = drawn.myIndex;
			}
			BlockSegment& next = blockSegments[targetIndex];
			// Make me the previous of the target
			next.prevIndex = drawn.myIndex;
			// Make target my next segment
			drawn.nextIndex = targetIndex;

			blockSegments[lastBlockSegment] = drawn;
			if (lastBlockSegment + 1< BLOCK_SEGMENT_AMOUNT)
			{
				lastBlockSegment += 1;
				break; // NOTE Done with this block
			}
			else
			{
				// BUG!
			printf("Too many but why\n");
				break;
			}
		}
		// 3. Try next one
		else if (drawn.start > target.end)
		{
			previousTargetIndex = targetIndex;
			// Take the index of the next segment
			targetIndex = target.nextIndex;
			//
			if (targetIndex < 0)
			{
				// This was the last one
				break;
			}
			continue;
		}
	}


	if (merged)
	{
		BlockSegment& target = blockSegments[targetIndex];
		int nextIndex = target.nextIndex;
		// See if touches the next one
		while (nextIndex >= 0)
		{
			// 4. Check if merge reaches next one
			BlockSegment& nextSegment = blockSegments[nextIndex];
			if (target.OverlapsWith(nextSegment))
			{
				nextSegment.MergeInto(target);
				// 5.
				nextSegment.Drop();
				nextIndex = nextSegment.nextIndex;
			}
			else
			{
				break;
			}
		}
	}
}
