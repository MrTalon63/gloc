FROM node:lts-alpine

WORKDIR /app
COPY package.json tsconfig.json ./
RUN npm install
COPY src ./src
RUN npm run build

EXPOSE 5000
EXPOSE 3000

CMD ["npm", "start"]