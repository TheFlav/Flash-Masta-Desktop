DROP TABLE IF EXISTS Games_Name_ind;
DROP TABLE IF EXISTS Games_Meta_ind;
DROP INDEX IF EXISTS Games_Hash_ind;
DROP TABLE IF EXISTS Games;

CREATE TABLE Games (
  ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `Hash` INTEGER,
  GameID INTEGER,
  GameVersion INTEGER,
  StartupAddress INTEGER,
  MinSystem INTEGER,
  License TEXT,
  CartName TEXT,
  GameName TEXT
);

CREATE INDEX Games_Hash_ind ON Games (`Hash`);
CREATE INDEX Games_Meta_ind ON Games (StartupAddress, GameID, GameVersion, MinSystem);
CREATE INDEX Games_Name_ind ON Games (CartName);
