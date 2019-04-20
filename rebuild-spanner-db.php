<?php
/*
 * usage
 *
	> export GOOGLE_APPLICATION_CREDENTIALS=/path/to/your-gcp-service-account.json
	> cat mysql-dump.sql | ./mysqldump2spanner.exe -D > spanner-input.sql
	> php rebuild-spanner-db.php [your spanner instance id] [your spanner database id] spanner-input.sql
 *
 */

/*
 * prepare library
 *
	> curl -sS https://getcomposer.org/installer | php
	> git clone https://github.com/googleapis/google-cloud-php.git
	> pushd ./google-cloud-php/
	> ../composer.phar require google/cloud-spanner
	> popd
 *
 */
require_once __DIR__ . '/google-cloud-php/vendor/autoload.php';

# Imports the Google Cloud client library
use Google\Cloud\Spanner\SpannerClient;
use Google\Cloud\Spanner\Database;
use Google\Cloud\Spanner\Transaction;

class GotoExit extends Exception { }

function init_database($instanceId, $databaseId)
{
	$client = new SpannerClient();

	$inst = $client->instance($instanceId);
	$db = $inst->database($databaseId);

	$db_exists = $db->exists();

	if ($db_exists)
	{
		$ans = readline("Do you really want to drop the '${databaseId}' database [y/N] ");
		if ($ans == 'y')
		{
			$db->drop();

			echo 'Database has been deleted.' . PHP_EOL;

			$db_exists = false;
		}
		else
		{
			$ans = readline('Do you want to continue [y/N] ');

			if ($ans != 'y')
			{
				throw new GotoExit('Cancelled.');
			}
		}
	}

	if (! $db_exists)
	{
		echo 'Waiting for the database to be created... ';

		$operation = $db->create();
		$operation->pollUntilComplete();

		echo 'done.' . PHP_EOL;
	}

	return $db;
}

function exec_sql($db, $sql)
{

	echo PHP_EOL . "\t# " . rtrim(str_replace(PHP_EOL, ' ', substr($sql, 0, 40)));
	echo (strlen($sql) > 40) ? '...' : '';
	echo PHP_EOL . PHP_EOL;

	if (preg_match('/^[[:space:]]*(create|drop|alter)/i', $sql))
	{
		echo 'Waiting for DDL to complete... ';

		$operation = $db->updateDdl($sql);
		$operation->pollUntilComplete();

		echo 'done.' . PHP_EOL;
	}
	else if (preg_match('/^[[:space:]]*(insert|update|delete)/i', $sql))
	{
		$db->runTransaction(function (Transaction $t) use ($sql)
		{
			echo 'Execute DML... ';

			$modifiedRowCount = $t->executeUpdate($sql);
			$t->commit();

			echo "${modifiedRowCount} row" . PHP_EOL;
		});
	}
	else
	{
		throw new Exception("unknown statement: ${sql}");
	}
}

function import_file($db, $fp)
{
	$sql = '';

	while ($line = fgets($fp))
	{
		$line = rtrim($line);

		if ($line === ';')
		{
			exec_sql($db, $sql);

			$sql = '';
		}
		else
		{
			$sql .= $line . PHP_EOL;
		}
	}
}

function Main($instanceId, $databaseId, $fp)
{
	$db = init_database($instanceId, $databaseId);

	import_file($db, $fp);
}

$rc = 0;

try
{
	if ($argc != 4)
	{
		throw new Exception("Usage: ${argv[0]} instance-id database-id input-file.sql");
	}

	if (! file_exists($argv[3]))
	{
		throw new Exception($argv[3] . ': file not found');
	}

	$fp = fopen($argv[3], 'r');
	if (! $fp)
	{
		throw new Exception($argv[3] . ': file open error');
	}

	Main($argv[1], $argv[2], $fp);

	echo PHP_EOL . 'all done.' . PHP_EOL;
}
catch (GotoExit $ex)
{
	echo PHP_EOL . $ex->getMessage() . PHP_EOL;

	$rc = 2;
}
catch (Exception $ex)
{
	echo 'MESSAGE: ' . $ex->getMessage() . PHP_EOL;
	echo 'POSITION: ' . $ex->getFile() . '(' . $ex->getLine() . ')' . PHP_EOL;
	echo str_repeat('-', 100) . PHP_EOL;
	echo 'TRACE' . PHP_EOL . PHP_EOL;
	echo $ex->getTraceAsString() . PHP_EOL;

	$rc = 1;
}

if (isset($fp))
{
	fclose($fp);
}

exit ($rc);

// EOF

