package bg.alex_iii.GLES;

import java.util.LinkedList;
import java.util.ListIterator;

public class GLESAllocator {
	protected int mSize;
	protected int mFree;
	protected LinkedList<Block> mBlocks;

	class Block {
		int mStart, mEnd;
		boolean mFree;
		
		Block(int start, int end, boolean free) {
			mStart = start;
			mEnd = end;
			mFree = free;
		}
		
		int getSize() {
			return mEnd - mStart;
		}
	}
	
	public GLESAllocator(int size) {
		mSize = size;
		mBlocks = new LinkedList<Block>();
		clear();
	}
	
	public int alloc(int size) {
		ListIterator<Block> it = findFree(size);
		if (it == null)
			return -1;
		Block b = it.previous();
		int start = b.mStart;
		int newEnd = b.mStart + size;
		if (newEnd < b.mEnd) {
			Block newPrev = new Block(start, newEnd, false);
			b.mStart = newEnd;
			it.add(newPrev);
		} else
			b.mFree = false;
		mFree -= size;
		return start;
	}
	
	public void free(int start) {
		ListIterator<Block> it = findStarting(start);
		assert it != null;
		if (it == null)
			return;
		Block b = it.previous();
		b.mFree = true;
		mFree += b.getSize();
		if (it.hasPrevious()) {
			Block prev = it.previous();
			if (prev.mFree) {
				b.mStart = prev.mStart;
				it.remove();
			} else
				it.next();
		}
		it.next();
		if (it.hasNext()) {
			Block next = it.next();
			if (next.mFree) {
				b.mEnd = next.mEnd;
				it.remove();
			}
		}
	}
	
	public void clear() {
		mFree = mSize;
		mBlocks.clear();
		mBlocks.add(new Block(0, mSize, true));
	}
	
	ListIterator<Block> findFree(int size) {
		ListIterator<Block> it = mBlocks.listIterator();
		boolean found = false;
		while (it.hasNext()) {
			Block b = it.next();
			if (b.mFree && b.getSize() >= size) {
				found = true;
				break;
			}
		}
		if (!found)
			return null;
		return it;
	}
	
	ListIterator<Block> findStarting(int start) {
		ListIterator<Block> it = mBlocks.listIterator();
		boolean found = false;
		while (it.hasNext()) {
			Block b = it.next();
			if (b.mStart == start) {
				assert !b.mFree;
				found = true;
				break;
			}
		}
		if (!found)
			return null;
		return it;
	}
}
