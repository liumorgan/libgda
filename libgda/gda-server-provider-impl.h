/*
 * Copyright (C) 2001 - 2004 Rodrigo Moya <rodrigo@gnome-db.org>
 * Copyright (C) 2002 - 2003 Gonzalo Paniagua Javier <gonzalo@ximian.com>
 * Copyright (C) 2004 - 2005 Alan Knowles <alank@src.gnome.org>
 * Copyright (C) 2005 Bas Driessen <bas.driessen@xobas.com>
 * Copyright (C) 2005 - 2014 Vivien Malerba <malerba@gnome-db.org>
 * Copyright (C) 2006 - 2008 Murray Cumming <murrayc@murrayc.com>
 * Copyright (C) 2007 Armin Burgmeier <armin@openismus.com>
 * Copyright (C) 2013 Daniel Espinosa <esodan@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

#ifndef __GDA_SERVER_PROVIDER_IMPL_H__
#define __GDA_SERVER_PROVIDER_IMPL_H__

#include <libgda/gda-server-operation.h>
#include <libgda/gda-connection.h>
#include <libgda/gda-data-model.h>
#include <libgda/gda-quark-list.h>
#include <libgda/gda-statement.h>
#include <libgda/gda-meta-store.h>
#include <libgda/gda-xa-transaction.h>
#include <libgda/thread-wrapper/gda-worker.h>

G_BEGIN_DECLS

/**
 * GdaServerProviderFunctionsType: (skip)
 *
 * Represents the different types of sets of virtual functions which can be implemented for each provider
 */
typedef enum {
	GDA_SERVER_PROVIDER_FUNCTIONS_BASE,
	GDA_SERVER_PROVIDER_FUNCTIONS_META,
	GDA_SERVER_PROVIDER_FUNCTIONS_XA,

	GDA_SERVER_PROVIDER_FUNCTIONS_MAX
} GdaServerProviderFunctionsType;


/*
 * May only be called once for each value of @type */
void gda_server_provider_set_impl_functions (GdaServerProviderClass *klass,
					     GdaServerProviderFunctionsType type, gpointer functions_set);
gpointer gda_server_provider_get_impl_functions_for_class (GObjectClass *klass, GdaServerProviderFunctionsType type);

/**
 * GdaServerProviderBase: (skip)
 * @get_name:
 * @get_version:
 *
 * Functions implementing basic features.
 *
 * A pointer to this structure is returned by _gda_server_provider_get_impl_functions() when requesting
 * @GDA_SERVER_PROVIDER_FUNCTIONS_BASE functions.
 */
typedef struct {
	/* functions called from any thread */
	const gchar        *(* get_name)              (GdaServerProvider *provider);
	const gchar        *(* get_version)           (GdaServerProvider *provider);
	const gchar        *(* get_server_version)    (GdaServerProvider *provider, GdaConnection *cnc);
	gboolean            (* supports_feature)      (GdaServerProvider *provider, GdaConnection *cnc,
						       GdaConnectionFeature feature);
	GdaWorker          *(* create_worker)         (GdaServerProvider *provider, gboolean for_cnc);
	GdaConnection      *(* create_connection)     (GdaServerProvider *provider); /* may be NULL */
	GdaSqlParser       *(* create_parser)         (GdaServerProvider *provider, GdaConnection *cnc); /* may be NULL */
	GdaDataHandler     *(* get_data_handler)      (GdaServerProvider *provider, GdaConnection *cnc, /* may be NULL */
						       GType g_type, const gchar *dbms_type);
	const gchar        *(* get_def_dbms_type)     (GdaServerProvider *provider, GdaConnection *cnc, GType g_type); /* may be NULL */
	gboolean            (* supports_operation)    (GdaServerProvider *provider, GdaConnection *cnc,
						       GdaServerOperationType type, GdaSet *options);
	GdaServerOperation *(* create_operation)      (GdaServerProvider *provider, GdaConnection *cnc,
						       GdaServerOperationType type, GdaSet *options, GError **error);
	gchar              *(* render_operation)      (GdaServerProvider *provider, GdaConnection *cnc,
						       GdaServerOperation *op, GError **error);
	/**
	 * statement_to_sql:
	 * @provider: a #GdaServerProvider
	 * @cnc: a #GdaConnection
	 * @stmt: a #GdaStatement
	 * @params: (allow-none): a #GdaSet object (which can be obtained using gda_statement_get_parameters()), or %NULL
	 * @flags: SQL rendering flags, as #GdaStatementSqlFlag OR'ed values
	 * @params_used: (allow-none) (element-type Gda.Holder) (out) (transfer container): a place to store the list of individual #GdaHolder objects within @params which have been used
	 * @error: a place to store errors, or %NULL
	 *
	 * Renders @stmt as an SQL statement, adapted to the SQL dialect used by @cnc
	 *
	 * Returns: a new string, or %NULL if an error occurred
	 */
	gchar              *(* statement_to_sql)     (GdaServerProvider *provider, GdaConnection *cnc,
						      GdaStatement *stmt, GdaSet *params, GdaStatementSqlFlag flags,
						      GSList **params_used, GError **error);
	/**
	 * identifier_quote:
	 * @provider: a #GdaServerProvider
	 * @cnc: (allow-none): a #GdaConnection, or %NULL
	 * @id: a string
	 * @for_meta_store: if %TRUE, then the result have to respect the #GdaMetaStore convention
	 * @force_quotes: if %TRUE, then quotes have to be added
	 *
	 * Create a new string in which @id can be used in an SQL statement, for example by adding quotes if
	 * it is a reserved keyword, or if it is case sensitive.
	 *
	 * If not %NULL, @cnc can either be opened or closed.
	 *
	 * Returns: (transfer full): a new string
	 */
	gchar              *(* identifier_quote)     (GdaServerProvider *provider, GdaConnection *cnc, /* may be NULL */
						      const gchar *id,
						      gboolean for_meta_store, gboolean force_quotes);

	GdaSqlStatement    *(* statement_rewrite)    (GdaServerProvider *provider, GdaConnection *cnc,
						      GdaStatement *stmt, GdaSet *params, GError **error);
	/**
	 * open_connection:
	 * @provider: a #GdaServerProvider
	 * @cnc: a #GdaConnection
	 * @params: (transfer none): a #GdaQuarkList containing the connection parameters (HOST, DATABASE, etc.)
	 * @auth: (transfer none) (allow-none): a #GdaQuarkList containing the connection authentification parameters (USERNAME, PASSWORD, etc.), or %NULL
	 *
	 * Open the connection. @params and @auth must be left unchanged.
	 *
	 * Returns: %TRUE if the connection was opened.
	 */
	gboolean      (* open_connection)       (GdaServerProvider *provider, GdaConnection *cnc,
						 GdaQuarkList *params, GdaQuarkList *auth);
	gboolean      (* prepare_connection)    (GdaServerProvider *provider, GdaConnection *cnc,
						 GdaQuarkList *params, GdaQuarkList *auth);
	gboolean      (* close_connection)      (GdaServerProvider *provider, GdaConnection *cnc);
	gchar        *(* escape_string)         (GdaServerProvider *provider, GdaConnection *cnc, const gchar *str); /* may be NULL */
	gchar        *(* unescape_string)       (GdaServerProvider *provider, GdaConnection *cnc, const gchar *str); /* may be NULL */
	gboolean      (* perform_operation)     (GdaServerProvider *provider, GdaConnection *cnc, /* may be NULL */
						 GdaServerOperation *op, GError **error);
	gboolean      (* begin_transaction)     (GdaServerProvider *provider, GdaConnection *cnc,
						 const gchar *name, GdaTransactionIsolation level, GError **error);
	gboolean      (* commit_transaction)    (GdaServerProvider *provider, GdaConnection *cnc,
						 const gchar *name, GError **error);
	gboolean      (* rollback_transaction)  (GdaServerProvider *provider, GdaConnection *cnc,
						 const gchar *name, GError **error);
	gboolean      (* add_savepoint)         (GdaServerProvider *provider, GdaConnection *cnc,
						 const gchar *name, GError **error);
	gboolean      (* rollback_savepoint)    (GdaServerProvider *provider, GdaConnection *cnc,
						 const gchar *name, GError **error);
	gboolean      (* delete_savepoint)      (GdaServerProvider *provider, GdaConnection *cnc,
						 const gchar *name, GError **error);
	gboolean      (* statement_prepare)     (GdaServerProvider *provider, GdaConnection *cnc,
						 GdaStatement *stmt, GError **error);
	GObject      *(* statement_execute)     (GdaServerProvider *provider, GdaConnection *cnc,
						 GdaStatement *stmt, GdaSet *params,
						 GdaStatementModelUsage model_usage,
						 GType *col_types, GdaSet **last_inserted_row, GError **error);

	/*< private >*/
	/* Padding for future expansion */
	void (*_gda_reserved11) (void);
	void (*_gda_reserved12) (void);
	void (*_gda_reserved13) (void);
	void (*_gda_reserved14) (void);
} GdaServerProviderBase;


/**
 * GdaServerProviderMeta: (skip)
 * @_info: 
 * @_btypes: 
 * @_udt: 
 * @udt: 
 * @_udt_cols: 
 * @udt_cols: 
 * @_enums: 
 * @enums: 
 * @_domains: 
 * @domains: 
 * @_constraints_dom: 
 * @constraints_dom: 
 * @_el_types: 
 * @el_types: 
 * @_collations: 
 * @collations: 
 * @_character_sets: 
 * @character_sets: 
 * @_schemata: 
 * @schemata: 
 * @_tables_views: 
 * @tables_views: 
 * @_columns: 
 * @columns: 
 * @_view_cols: 
 * @view_cols: 
 * @_constraints_tab: 
 * @constraints_tab: 
 * @_constraints_ref: 
 * @constraints_ref: 
 * @_key_columns: 
 * @key_columns: 
 * @_check_columns: 
 * @check_columns: 
 * @_triggers: 
 * @triggers: 
 * @_routines: 
 * @routines: 
 * @_routine_col: 
 * @routine_col: 
 * @_routine_par: 
 * @routine_par: 
 * @_indexes_tab: 
 * @indexes_tab: 
 * @_index_cols: 
 * @index_cols:
 *
 * These methods must be implemented by providers to update a connection's associated metadata (in a 
 * #GdaMetaStore object), see the <link linkend="prov-metadata">Virtual methods for providers/Methods - metadata</link>
 * for more information.
 */
typedef struct {
	/* _information_schema_catalog_name */
	gboolean (*_info)            (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);

	/* _builtin_data_types */
	gboolean (*_btypes)          (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);

	/* _udt */
	gboolean (*_udt)             (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*udt)              (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *udt_catalog, const GValue *udt_schema);

	/* _udt_columns */
	gboolean (*_udt_cols)        (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*udt_cols)         (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *udt_catalog, const GValue *udt_schema, const GValue *udt_name);

	/* _enums */
	gboolean (*_enums)           (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*enums)            (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *udt_catalog, const GValue *udt_schema, const GValue *udt_name);

	/* _domains */
	gboolean (*_domains)         (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*domains)          (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *domain_catalog, const GValue *domain_schema);

	/* _domain_constraints */
	gboolean (*_constraints_dom) (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*constraints_dom)  (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *domain_catalog, const GValue *domain_schema, const GValue *domain_name);

	/* _element_types */
	gboolean (*_el_types)        (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*el_types)         (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *specific_name);

	/* _collations */
	gboolean (*_collations)       (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*collations)        (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				       const GValue *collation_catalog, const GValue *collation_schema, 
				       const GValue *collation_name_n);

	/* _character_sets */
	gboolean (*_character_sets)  (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*character_sets)   (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *chset_catalog, const GValue *chset_schema, const GValue *chset_name_n);

	/* _schemata */
	gboolean (*_schemata)        (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*schemata)         (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error, 
				      const GValue *catalog_name, const GValue *schema_name_n);

	/* _tables or _views */
	gboolean (*_tables_views)    (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*tables_views)     (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name_n);

	/* _columns */
	gboolean (*_columns)         (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*columns)          (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name);

	/* _view_column_usage */
	gboolean (*_view_cols)       (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*view_cols)        (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *view_catalog, const GValue *view_schema, const GValue *view_name);

	/* _table_constraints */
	gboolean (*_constraints_tab) (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*constraints_tab)  (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error, 
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name,
				      const GValue *constraint_name_n);

	/* _referential_constraints */
	gboolean (*_constraints_ref) (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*constraints_ref)  (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name, 
				      const GValue *constraint_name);

	/* _key_column_usage */
	gboolean (*_key_columns)     (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*key_columns)      (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name, 
				      const GValue *constraint_name);

	/* _check_column_usage */
	gboolean (*_check_columns)   (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*check_columns)    (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name, 
				      const GValue *constraint_name);

	/* _triggers */
	gboolean (*_triggers)        (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*triggers)         (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name);

	/* _routines */
	gboolean (*_routines)       (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*routines)        (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				     const GValue *routine_catalog, const GValue *routine_schema, 
				     const GValue *routine_name_n);

	/* _routine_columns */
	gboolean (*_routine_col)     (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*routine_col)      (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *rout_catalog, const GValue *rout_schema, const GValue *rout_name);

	/* _parameters */
	gboolean (*_routine_par)     (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*routine_par)      (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *rout_catalog, const GValue *rout_schema, const GValue *rout_name);
	/* _table_indexes */
	gboolean (*_indexes_tab)     (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*indexes_tab)      (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error, 
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name,
				      const GValue *index_name_n);

	/* _index_column_usage */
	gboolean (*_index_cols)      (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error);
	gboolean (*index_cols)       (GdaServerProvider *prov, GdaConnection *cnc, GdaMetaStore *meta, GdaMetaContext *ctx, GError **error,
				      const GValue *table_catalog, const GValue *table_schema, const GValue *table_name, const GValue *index_name);
	
	/*< private >*/
	/* Padding for future expansion */
	void (*_gda_reserved1) (void);
	void (*_gda_reserved2) (void);
	void (*_gda_reserved3) (void);
	void (*_gda_reserved4) (void);
	void (*_gda_reserved5) (void);
	void (*_gda_reserved6) (void);
	void (*_gda_reserved7) (void);
	void (*_gda_reserved8) (void);
	void (*_gda_reserved9) (void);
	void (*_gda_reserved10) (void);
	void (*_gda_reserved11) (void);
	void (*_gda_reserved12) (void);
} GdaServerProviderMeta;

typedef enum {
	/* _information_schema_catalog_name */
	GDA_SERVER_META__INFO = 0,

	/* _builtin_data_types */
	GDA_SERVER_META__BTYPES = 1,

	/* _udt */
	GDA_SERVER_META__UDT = 2,
	GDA_SERVER_META_UDT = 3,

	/* _udt_columns */
	GDA_SERVER_META__UDT_COLS = 4,
	GDA_SERVER_META_UDT_COLS = 5,

	/* _enums */
	GDA_SERVER_META__ENUMS = 6,
	GDA_SERVER_META_ENUMS = 7,

	/* _domains */
	GDA_SERVER_META__DOMAINS = 8,
	GDA_SERVER_META_DOMAINS = 9,

	/* _domain_constraints */
	GDA_SERVER_META__CONSTRAINTS_DOM = 10,
	GDA_SERVER_META_CONSTRAINTS_DOM = 11,

	/* _element_types */
	GDA_SERVER_META__EL_TYPES = 12,
	GDA_SERVER_META_EL_TYPES = 13,

	/* _collations */
	GDA_SERVER_META__COLLATIONS = 14,
	GDA_SERVER_META_COLLATIONS = 15,

	/* _character_sets */
	GDA_SERVER_META__CHARACTER_SETS = 16,
	GDA_SERVER_META_CHARACTER_SETS = 17,

	/* _schemata */
	GDA_SERVER_META__SCHEMATA = 18,
	GDA_SERVER_META_SCHEMATA = 19,

	/* _tables or _views */
	GDA_SERVER_META__TABLES_VIEWS = 20,
	GDA_SERVER_META_TABLES_VIEWS = 21,

	/* _columns */
	GDA_SERVER_META__COLUMNS = 22,
	GDA_SERVER_META_COLUMNS = 23,

	/* _view_column_usage */
	GDA_SERVER_META__VIEW_COLS = 24,
	GDA_SERVER_META_VIEW_COLS = 25,

	/* _table_constraints */
	GDA_SERVER_META__CONSTRAINTS_TAB = 26,
	GDA_SERVER_META_CONSTRAINTS_TAB = 27,

	/* _referential_constraints */
	GDA_SERVER_META__CONSTRAINTS_REF = 28,
	GDA_SERVER_META_CONSTRAINTS_REF = 29,

	/* _key_column_usage */
	GDA_SERVER_META__KEY_COLUMNS = 30,
	GDA_SERVER_META_KEY_COLUMNS = 31,

	/* _check_column_usage */
	GDA_SERVER_META__CHECK_COLUMNS = 32,
	GDA_SERVER_META_CHECK_COLUMNS = 33,

	/* _triggers */
	GDA_SERVER_META__TRIGGERS = 34,
	GDA_SERVER_META_TRIGGERS = 35,

	/* _routines */
	GDA_SERVER_META__ROUTINES = 36,
	GDA_SERVER_META_ROUTINES = 37,

	/* _routine_columns */
	GDA_SERVER_META__ROUTINE_COL = 38,
	GDA_SERVER_META_ROUTINE_COL = 39,

	/* _parameters */
	GDA_SERVER_META__ROUTINE_PAR = 40,
	GDA_SERVER_META_ROUTINE_PAR = 41,

	/* _table_indexes */
	GDA_SERVER_META__INDEXES_TAB = 42,
	GDA_SERVER_META_INDEXES_TAB = 43,

	/* _index_column_usage */
	GDA_SERVER_META__INDEX_COLS = 44,
	GDA_SERVER_META_INDEX_COLS = 45,
} GdaServerProviderMetaType;


/**
 * GdaServerProviderXa: (skip)
 * @xa_start:
 * @xa_end:
 * @xa_prepare:
 * @xa_commit:
 * @xa_rollback:
 * @xa_recover:
 *
 * Functions implementing distributed transactions.
 */
typedef struct {
	gboolean (*xa_start)    (GdaServerProvider *prov, GdaConnection *cnc, const GdaXaTransactionId *trx, GError **error);

	gboolean (*xa_end)      (GdaServerProvider *prov, GdaConnection *cnc, const GdaXaTransactionId *trx, GError **error);
	gboolean (*xa_prepare)  (GdaServerProvider *prov, GdaConnection *cnc, const GdaXaTransactionId *trx, GError **error);

	gboolean (*xa_commit)   (GdaServerProvider *prov, GdaConnection *cnc, const GdaXaTransactionId *trx, GError **error);
	gboolean (*xa_rollback) (GdaServerProvider *prov, GdaConnection *cnc, const GdaXaTransactionId *trx, GError **error);

	GList   *(*xa_recover)  (GdaServerProvider *prov, GdaConnection *cnc, GError **error);

	/*< private >*/
	/* Padding for future expansion */
	void (*_gda_reserved1) (void);
	void (*_gda_reserved2) (void);
	void (*_gda_reserved3) (void);
	void (*_gda_reserved4) (void);
} GdaServerProviderXa;

G_END_DECLS

#endif
