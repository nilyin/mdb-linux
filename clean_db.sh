#!/bin/bash
echo "Cleaning corrupted database files..."
rm -rf /app/data/rowdata/*
rm -rf /app/data/trlog/*
rm -f /app/data/metainf.mdb*
rm -f /app/data/tables.mdb*
echo "Database files cleaned. Restart the server to create fresh database."
