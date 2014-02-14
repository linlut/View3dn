//FILE: dlinkedlist.h
//

#ifndef __INC_DLINKEDLIST123_H__
#define __INC_DLINKEDLIST123_H__
class CDLinkedListItem
{
public:
	int m_nIndex;
	CDLinkedListItem * m_pPre;
	CDLinkedListItem * m_pNext;

private:
	void _assignNewIndex(CDLinkedListItem * phead, const int index)
	{
		CDLinkedListItem *p = phead;
		do{
			p->m_nIndex = index;
			p=p->m_pNext;
		}while (p!=phead);	
	}

public:
	CDLinkedListItem(const int index)
	{
		m_nIndex = index;
		m_pPre = m_pNext = this;
	}

	CDLinkedListItem(void)
	{
		m_nIndex = -1;
		m_pPre = m_pNext = this;
	}

	bool sameList(const CDLinkedListItem *p)
	{
		bool r = false;
		if (this->m_nIndex == p->m_nIndex)
			r = true;
		return r;
	}

	void mergeList(CDLinkedListItem *p)
	{
		if (sameList(p)) return;

		//asign the index;
		CDLinkedListItem *p1 = this;
		CDLinkedListItem *p2 = p;
		if (p1->m_nIndex<p2->m_nIndex)
			_assignNewIndex(p2, p1->m_nIndex);
		else
			_assignNewIndex(this, p2->m_nIndex);

		//adjust pointers to merge clusters;
		CDLinkedListItem *p1next = p1->m_pNext;
		CDLinkedListItem *p2pre = p2->m_pPre;
		p1->m_pNext = p2;
		p2->m_pPre = p1;
		p1next->m_pPre = p2pre;
		p2pre->m_pNext = p1next;
	}
};

#endif