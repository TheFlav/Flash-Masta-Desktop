DROP INDEX IF EXISTS Games_Hash_ind;
DROP TABLE IF EXISTS Games;

CREATE TABLE Games (
  ID INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
  `Hash` INTEGER,
  GameID INTEGER,
  GameName TEXT,
  Developer TEXT,
  RomSize INTEGER,
  SaveSize INTEGER,
  MinSystem INTEGER,
  MapperVersion INTEGER,
  RTC INTEGER,
  `Checksum` INTEGER,
  Flags INTEGER
);

CREATE INDEX Games_Hash_ind ON Games (`Hash`);
