# Dockerfile
FROM node:18-alpine

# Tentukan folder kerja di dalam container
WORKDIR /app

# Copy package.json terlebih dahulu untuk efisiensi cache layer Docker
COPY package.json ./

# Install dependensi Node.js
RUN npm install

# Copy seluruh kode program ke dalam container
COPY cache-token.js get-token.js ./

# Secara default tidak menjalankan command apa pun, 
# karena command spesifik akan diatur langsung di docker-compose.yml