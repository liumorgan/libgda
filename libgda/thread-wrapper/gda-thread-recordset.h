/* GDA Library
 * Copyright (C) 2009 The GNOME Foundation.
 *
 * AUTHORS:
 *         Vivien Malerba <malerba@gnome-db.org>
 *
 * This Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this Library; see the file COPYING.LIB.  If not,
 * write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GDA_THREAD_RECORDSET_H__
#define __GDA_THREAD_RECORDSET_H__

#include <libgda/gda-connection.h>
#include <libgda/gda-data-select.h>
#include <thread-wrapper/gda-thread-wrapper.h>

G_BEGIN_DECLS

#define GDA_TYPE_THREAD_RECORDSET            (_gda_thread_recordset_get_type())
#define GDA_THREAD_RECORDSET(obj)            (G_TYPE_CHECK_INSTANCE_CAST (obj, GDA_TYPE_THREAD_RECORDSET, GdaThreadRecordset))
#define GDA_THREAD_RECORDSET_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST (klass, GDA_TYPE_THREAD_RECORDSET, GdaThreadRecordsetClass))
#define GDA_IS_THREAD_RECORDSET(obj)         (G_TYPE_CHECK_INSTANCE_TYPE (obj, GDA_TYPE_THREAD_RECORDSET))
#define GDA_IS_THREAD_RECORDSET_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GDA_TYPE_THREAD_RECORDSET))

typedef struct _GdaThreadRecordset        GdaThreadRecordset;
typedef struct _GdaThreadRecordsetClass   GdaThreadRecordsetClass;
typedef struct _GdaThreadRecordsetPrivate GdaThreadRecordsetPrivate;

struct _GdaThreadRecordset {
	GdaDataSelect                  model;
	GdaThreadRecordsetPrivate     *priv;
};

struct _GdaThreadRecordsetClass {
	GdaDataSelectClass             parent_class;
};

GType         _gda_thread_recordset_get_type  (void) G_GNUC_CONST;
GdaDataModel *_gda_thread_recordset_new       (GdaConnection *cnc, GdaThreadWrapper *wrapper, 
					       GdaDataModel *sub_model);

G_END_DECLS

#endif