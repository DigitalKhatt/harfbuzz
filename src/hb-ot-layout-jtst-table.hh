/*
 * Copyright (c) 2015-2020 Amine Anane. http: //digitalkhatt/license
 * This file is part of DigitalKhatt.
 *
 * DigitalKhatt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * DigitalKhatt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.

 * You should have received a copy of the GNU Affero General Public License
 * along with DigitalKhatt. If not, see
 * <https: //www.gnu.org/licenses />.
*/

#ifndef HB_OT_LAYOUT_JTST_TABLE_HH
#define HB_OT_LAYOUT_JTST_TABLE_HH

#include "hb-open-type.hh"
#include "hb-ot-layout-gpos-table.hh"


#define HB_OT_TAG_JTST HB_TAG ('J', 'T', 'S', 'T')

namespace OT {

typedef IndexArray JtstLookupList;

struct JtstStep
{
  bool isSubtitution () const { return (flags & 1); }

  const JtstLookupList &get_lookups () const { return lookupList; }

  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (true);
  }

  protected:
  HBUINT32 flags;
  JtstLookupList lookupList;
};

struct JtstSteps : OffsetListOf<JtstStep>
{
  bool sanitize (hb_sanitize_context_t *c,
		 const Record_sanitize_closure_t * = nullptr) const
  {
    TRACE_SANITIZE (this);
    return_trace (OffsetListOf<JtstStep>::sanitize (c));
  }
};


struct JTST
{
  static constexpr hb_tag_t tableTag = HB_OT_TAG_JTST;

  const JtstSteps& get_stretch_steps () const { return this + stretchSteps; }
  const JtstSteps &get_shrink_steps () const { return this + shrinkSteps; }

  bool sanitize (hb_sanitize_context_t *c) const
  {
    TRACE_SANITIZE (this);
    return_trace (version.sanitize (c)
      && likely (version.major == 1)
      && stretchSteps.sanitize (c, this)
      && shrinkSteps.sanitize (c, this));
  }

  protected:
  FixedVersion<> version; /* Version of the JTST table--initially set
			   * to 0x00010000u */

  OffsetTo<JtstSteps> stretchSteps;

  OffsetTo<JtstSteps> shrinkSteps;

  public:
    DEFINE_SIZE_STATIC (8);
};

} /* namespace OT */

#endif /* HB_OT_LAYOUT_JTST_TABLE_HH */
