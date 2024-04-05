CREATE TABLE Users (
    login VARCHAR(100) PRIMARY KEY,
    password VARCHAR(100)
);

CREATE TABLE Messages (
    theme VARCHAR(100),
    message VARCHAR(100),
    sender VARCHAR(100) NOT NULL,
    recipient VARCHAR(100) NOT NULL,
    date TIMESTAMP(0) WITH TIME ZONE,
    FOREIGN KEY (sender) REFERENCES Users(login),
    FOREIGN KEY (recipient) REFERENCES Users(login)
);