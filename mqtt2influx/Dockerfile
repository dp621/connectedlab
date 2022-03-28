FROM node:10.17.0-alpine
WORKDIR /usr/app
copy package.json .
RUN npm install --quiet
COPY . .
CMD [ "npm", "start" ]
