
# SENTITIVIY
``` c
    sc_event_finder& pos() const
    {
        if ( !m_pos_finder_p )
	{
	    m_pos_finder_p = new sc_event_finder_t<in_if_type>(
	        *this, &in_if_type::posedge_event );
	} 
	return *m_pos_finder_p;
    }

    // use for negative edge sensitivity

    sc_event_finder& neg() const
    {
        if ( !m_neg_finder_p )
	{
	    m_neg_finder_p = new sc_event_finder_t<in_if_type>(
	        *this, &in_if_type::negedge_event );
	} 
	return *m_neg_finder_p;
    }
```
