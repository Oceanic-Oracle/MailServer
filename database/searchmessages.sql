SELECT *
FROM messages
WHERE sender = $1 OR recipient = $1