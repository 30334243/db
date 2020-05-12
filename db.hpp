#pragma once
#include <Header.h>

class db
{
	QSqlDatabase mDb;
	QString mName;
	const QString TABLE = "_table";
public:
	// create db
	explicit db(const QString& name) :
		mName{ name }
	{
		mDb = QSqlDatabase::addDatabase("QSQLITE");
		mDb.setDatabaseName(mName + ".sqlite");
		if (!(mDb.open()))
		{
			QMessageBox::warning(0, "Error!", mDb.lastError().text());
		}
	}
	// create table
	template<typename... Args>
	bool createTable(const QString& table, Args&&... args)
	{
		QSqlQuery query(mDb);
		std::vector<std::pair<QString, QString>> input;
		(input.push_back(std::make_pair(std::forward<Args>(args).first, std::forward<Args>(args).second)), ...);
		QString out;
		size_t idx = 0;
		for (const auto& elm : input)
		{
			auto punctuation = (idx++ == (input.size() - 1)) ? "" : ", ";
			out += elm.first + " " + elm.second + punctuation;
		}
		QString str = "CREATE TABLE " + table + " (id INTEGER PRIMARY KEY AUTOINCREMENT, " + out + ");";
		if (!(query.exec(str)))
		{
			QMessageBox::warning(0, "Error!", "Не удалось создать таблицу!");
			mDb.close();
			return false;
		}
		return true;
	}
	template<typename... Args>
	void createTableMoreAndMore(const QString& table)
	{
		QSqlQuery query(mDb);
		QString str = "CREATE TABLE " + table + " ("
			"date_id INTEGER, "
			"sender_id INTEGER, "
			"recipient_id INTEGER, "
			"FOREIGN KEY (date_id) REFERENCES date(id) "
			"FOREIGN KEY (sender_id) REFERENCES sender(id) "
			"FOREIGN KEY (recipient_id) REFERENCES recipient(id))";
		if (!(query.exec(str)))
		{
			QMessageBox::warning(0, "Error!", mDb.lastError().text());
			mDb.close();
		}
	}
	// insert data
	template<typename... Args>
	bool insert(const QString& table, Args&&... args)
	{
		QSqlQuery query(mDb);
		// get parametrs
		std::vector<std::pair<QString, QString>> input;
		(input.push_back(std::make_pair(forward<Args>(args).first, forward<Args>(args).second)), ...);
		// formating parametrs
		QString one;
		QString values;
		size_t idx = 0;
		for (const auto& elm : input)
		{
			auto punctuation = (idx++ == (input.size() - 1)) ? "" : ", ";
			one += elm.first + punctuation;
			values += ":" + elm.first + punctuation;
		}
		query.prepare(
			"INSERT INTO " + table + " (" + one + ")"
			"VALUES(" + values + ")"
		);
		for (const auto& in : input)
		{
			query.bindValue(":" + in.first, in.second);
		}
		if (!(query.exec()))
		{
			QMessageBox::warning(0, "Error!", "Не удалось вставить данные в базу данных!");
			mDb.close();
			return false;
		}
		return true;
	}
	void insert2(const QString& table, const int& t, const int& s, const int& r)
	{
		QSqlQuery query(mDb);
		query.prepare(
			"INSERT INTO " + table + " (date_id, sender_id, recipient_id)"
			"VALUES (" + QString::number(t) + ", " + QString::number(s) + ", " + QString::number(r) + ");"
		);
		query.bindValue(":date_id", t);
		query.bindValue(":sender_id", s);
		query.bindValue(":recipient_id", r);
		if (!(query.exec()))
		{
			QMessageBox::warning(0, "Error!", "Не удалось вставить данные в базу данных!");
			mDb.close();
		}
	}
	// select
	template<typename... Args>
	std::vector<QString> select(Args&&... args)
	{
		QSqlQuery query(mDb);
		if (!(query.exec("SELECT * FROM " + TABLE)))
		{
			QMessageBox::warning(0, "Error!", mDb.lastError().text());
			return std::vector<QString>();
		}
		QSqlRecord rec = query.record();
		std::vector<QString> out;

		std::vector<QString> header_tab;
		(header_tab.push_back(std::forward<Args>(args)), ...);

		while (query.next())
		{
			for (const auto& elm : header_tab)
			{
				auto idx = rec.indexOf(elm);
				out.push_back(query.value(idx).toString());
			}
		}
		return out;
	}
	// get element
	template<typename... Args>
	std::vector<QString> get_element(const QString& id, Args&&... args)
	{
		QSqlQuery query(mDb);
		if (!(query.exec("SELECT * FROM " + TABLE + " WHERE id = '" + id + "'")))
		{
			QMessageBox::warning(0, "Error!", mDb.lastError().text());
			return std::vector<QString>();
		}
		QSqlRecord rec = query.record();
		std::vector<QString> out;

		std::vector<QString> header_tab;
		(header_tab.push_back(std::forward<Args>(args)), ...);

		while (query.next())
		{
			for (const auto& elm : header_tab)
			{
				auto idx = rec.indexOf(elm);
				out.push_back(query.value(idx).toString());
			}
		}
		return out;
	}
	int get_element2(const QString& table, const QString& header, const QString& elm)
	{
		QSqlQuery query(mDb);
		if (!(query.exec("SELECT * FROM " + table + " WHERE " + header + " = '" + elm + "'")))
		{
			QMessageBox::warning(0, "Error!", mDb.lastError().text());
			return 0;
		}
		QSqlRecord rec = query.record();

		query.next();
		auto idx{ rec.indexOf("id") };
		return query.value(idx).toInt();
	}
	// get element
	template<typename... Args>
	void get_element_str(const QString& id, std::vector<std::string>&& out, Args&&... args);
	// get last element
	QString get_last_element(const QString& table, const QString& header)
	{
		QSqlQuery query(mDb);
		if (!(query.exec("SELECT * FROM " + table + " WHERE ID = (SELECT MAX(ID) FROM " + table + ")")))
		{
			QMessageBox::warning(0, "Error!", "query.exec(...)");
			return QString{};
		}
		QSqlRecord rec = query.record();
		query.next();
		auto elm = query.value(rec.indexOf(header)).toString();
		return elm;
	}
	// get last element
	QString get_last_element(const QString& table)
	{
		QSqlQuery query(mDb);
		if (!(query.exec("SELECT * FROM " + table + " WHERE ID = (SELECT MAX(ID) FROM " + table + ")")))
		{
			QMessageBox::warning(0, "Error!", "query.exec(...)");
			return QString{};
		}
		QSqlRecord rec = query.record();
		query.next();
		auto elm = query.value(rec.indexOf("ID")).toString();
		return elm;
	}
	// find
	int find(const QString& table, const QString& header, const QString& text)
	{
		if (text.size() == 0)
		{
			QMessageBox::warning(0, "Error!", "Введите корректный запрос поиска!");
			return false;
		}
		QSqlQuery query(mDb);
		if (!(query.exec("SELECT * FROM " + table + " WHERE " + header + " = '" + text + "'")))
		{
			QMessageBox::warning(0, "Error!", "query.exec(...)");
			return false;
		}
		QSqlRecord rec = query.record();
		query.next();
		auto elm = query.value(rec.indexOf(header)).toString().size();
		return elm ? elm : 0;
	}
	// delete
	template<typename... Args>
	bool delete_row(Args&&... args);
	// table
	QStringList getTable()
	{
		return mDb.tables();
	}
};

// create db
//db::db(const QString& name) :
//	mName{ name }
//{
//	mDb = QSqlDatabase::addDatabase("QSQLITE");
//	mDb.setDatabaseName(mName + ".sqlite");
//	if (!(mDb.open())) {
//		QMessageBox::warning(0, "Error!", mDb.lastError().text());
//	}
//}
// create table
//template<typename... Args>
//bool db::createTable(Args&&... args) {
//	QSqlQuery query(mDb);
//	std::vector<std::pair<QString, QString>> input;
//	(input.push_back(std::make_pair(std::forward<Args>(args).first, std::forward<Args>(args).second)), ...);
//	QString out;
//	size_t idx = 0;
//	for (const auto& elm : input) {
//		auto punctuation = (idx++ == (input.size() - 1)) ? "" : ", ";
//		out += elm.first + " " + elm.second + punctuation;
//	}
//	QString str = "CREATE TABLE " + TABLE + " (id INTEGER PRIMARY KEY AUTOINCREMENT, " + out + ");";
//	if (!(query.exec(str))) {
//		QMessageBox::warning(0, "Error!", "Не удалось создать таблицу!");
//		mDb.close();
//		return false;
//	}
//	return true;
//}
//template<typename... Args>
//bool db::insert(Args&&... args) {
//	QSqlQuery query(mDb);
//	// get parametrs
//	std::vector<std::pair<QString, QString>> input;
//	(input.push_back(std::make_pair(forward<Args>(args).first, forward<Args>(args).second)), ...);
//	// formating parametrs
//	QString one;
//	QString values;
//	size_t idx = 0;
//	for (const auto& elm : input) {
//		auto punctuation = (idx++ == (input.size() - 1)) ? "" : ", ";
//		one += elm.first + punctuation;
//		values += ":" + elm.first + punctuation;
//	}
//	query.prepare(
//		"INSERT INTO " + TABLE + " (" + one + ")"
//		"VALUES(" + values + ")"
//	);
//	for (const auto& in : input) {
//		query.bindValue(":" + in.first, in.second);
//	}
//	if (!(query.exec())) {
//		QMessageBox::warning(0, "Error!", "Не удалось вставить данные в базу данных!");
//		mDb.close();
//		return false;
//	}
//	return true;
//}
// select
//template<typename... Args>
//std::vector<QString> db::select(Args&&... args) {
//	QSqlQuery query(mDb);
//	if (!(query.exec("SELECT * FROM " + TABLE))) {
//		QMessageBox::warning(0, "Error!", mDb.lastError().text());
//		return std::vector<QString>();
//	}
//	QSqlRecord rec = query.record();
//	std::vector<QString> out;
//
//	std::vector<QString> header_tab;
//	(header_tab.push_back(std::forward<Args>(args)), ...);
//
//	while (query.next()) {
//		for (const auto& elm : header_tab) {
//			auto idx = rec.indexOf(elm);
//			out.push_back(query.value(idx).toString());
//		}
//	}
//	return out;
//}
// get element
//template<typename... Args>
//std::vector<QString> db::get_element(const QString& id, Args&&... args) {
//	QSqlQuery query(mDb);
//	if (!(query.exec("SELECT * FROM " + TABLE + " WHERE id = '" + id + "'"))) {
//		QMessageBox::warning(0, "Error!", mDb.lastError().text());
//		return std::vector<QString>();
//	}
//	QSqlRecord rec = query.record();
//	std::vector<QString> out;
//
//	std::vector<QString> header_tab;
//	(header_tab.push_back(std::forward<Args>(args)), ...);
//
//	while (query.next()) {
//		for (const auto& elm : header_tab) {
//			auto idx = rec.indexOf(elm);
//			out.push_back(query.value(idx).toString());
//		}
//	}
//	return out;
//}
// get element
template<typename... Args>
void db::get_element_str(const QString& id, std::vector<std::string>&& out, Args&&... args)
{
	QSqlQuery query(mDb);
	if (!(query.exec("SELECT * FROM " + TABLE + " WHERE id = '" + id + "'")))
	{
		QMessageBox::warning(0, "Error!", mDb.lastError().text());
		return;
	}
	QSqlRecord rec = query.record();
	//vector<string> out;

	std::vector<QString> header_tab;
	(header_tab.push_back(forward<Args>(args)), ...);

	while (query.next())
	{
		for (const auto& elm : header_tab)
		{
			auto idx = rec.indexOf(elm);
			out.push_back(query.value(idx).toString().toStdString());
		}
	}
	return;
}
// get last element
//QString db::get_last_element(const QString& header) {
//	QSqlQuery query(mDb);
//	if (!(query.exec("SELECT * FROM " + TABLE + " WHERE ID = (SELECT MAX(ID) FROM " + TABLE + ")"))) {
//		QMessageBox::warning(0, "Error!", "query.exec(...)");
//		return false;
//	}
//	QSqlRecord rec = query.record();
//	query.next();
//	auto elm = query.value(rec.indexOf(header)).toString();
//	return elm;
//}
// get last element
//QString db::get_last_element() {
//	QSqlQuery query(mDb);
//	if (!(query.exec("SELECT * FROM " + TABLE + " WHERE ID = (SELECT MAX(ID) FROM " + TABLE + ")"))) {
//		QMessageBox::warning(0, "Error!", "query.exec(...)");
//		return false;
//	}
//	QSqlRecord rec = query.record();
//	query.next();
//	auto elm = query.value(rec.indexOf("ID")).toString();
//	return elm;
//}
// find
//bool db::find(const QString& header, const QString& text) {
//	if (text.size() == 0) {
//		QMessageBox::warning(0, "Error!", "Введите корректный запрос поиска!");
//		return false;
//	}
//	QSqlQuery query(mDb);
//	if (!(query.exec("SELECT * FROM " + TABLE + " WHERE " + header + " = '" + text + "'"))) {
//		QMessageBox::warning(0, "Error!", "query.exec(...)");
//		return false;
//	}
//	QSqlRecord rec = query.record();
//	query.next();
//	auto elm = query.value(rec.indexOf(header)).toString().size();
//	if (elm != 0) {
//		QMessageBox::warning(0, "Error!", "Данный элемент \"" + text + "\" присутствует в базе!");
//		return false;
//	}
//	return true;
//}
//QStringList db::getTable()
//{
//	return mDb.tables();
//}
// delete
template<typename... Args>
bool db::delete_row(Args&&... args)
{
	bool result = true;
	std::list<std::pair<QString, QString>> out;
	(out.push_back(make_pair(forward<Args>(args).first, forward<Args>(args).second)), ...);
	QSqlQuery query(mDb);
	for (const auto& elm : out)
	{
		if (!(query.exec("DELETE FROM " + TABLE + " WHERE " + elm.first + " = '" + elm.second + "'")))
		{
			QMessageBox::warning(0, "Error!", "Не удалось удалить строку " + elm.second + " заголовка таблицы " + elm.first + "!");
			result = false;
		}
	}
	return result;
}