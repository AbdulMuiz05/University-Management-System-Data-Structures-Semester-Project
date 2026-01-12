#include <iostream>
#include <string>
#include <ctime>
#include <climits>

using namespace std;

// ==========================================
//        FORWARD DECLARATIONS
// ==========================================
class PeopleDirectory;
class CampusMap;
class RoomAVLTree;
class ComplaintQueue;
class MessagingSystem;

// ==========================================
//        GLOBAL SHARED OBJECTS
// ==========================================
extern PeopleDirectory g_people;
extern CampusMap g_map;
extern RoomAVLTree g_rooms;
extern ComplaintQueue g_complaints;
extern MessagingSystem g_messaging;

// ==========================================
//        UTILITY FUNCTIONS
// ==========================================
string getCurrentTime() {
    time_t now = time(0);
    tm localTime;
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    char buffer[30];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &localTime);
    return string(buffer);
}

// ==========================================
//    HELPER CLASSES FOR MAP (BFS/DFS)
// ==========================================
struct MapNode {
    int data; // building index
    MapNode* next;
    MapNode(int d) : data(d), next(nullptr) {}
};

class MapQueue {
    MapNode* front;
    MapNode* rear;
public:
    MapQueue() { 
        front = rear = nullptr; 
    }
    bool isEmpty() {
        return front == nullptr;
    }
    void enqueue(int val) {
        MapNode* newNode = new MapNode(val);
        if (isEmpty()) {
            front = rear = newNode;
        }
        else {
            rear->next = newNode;
            rear = newNode;
        }
    }
    int dequeue() {
        if (isEmpty()) {
            return -1;
        }
        MapNode* temp = front;
        int val = temp->data;
        front = front->next;
        if (!front) {
            rear = nullptr;
        }
        delete temp;
        return val;
    }
};

class MapStack {
    MapNode* topNode;
public:
    MapStack() {
        topNode = nullptr;
    }
    bool isEmpty() {
        return topNode == nullptr;
    }
    void push(int val) {
        MapNode* newNode = new MapNode(val);
        newNode->next = topNode;
        topNode = newNode;
    }
    int pop() {
        if (isEmpty()) {
            return -1;
        }
        MapNode* temp = topNode;
        int val = temp->data;
        topNode = topNode->next;
        delete temp;
        return val;
    }
};

// ==========================================
//    MODULE E: MESSAGING (Stacks)
// ==========================================
struct MessageNode {
    string fromUser;
    string toUser;
    string messageText;
    string timestamp;
    MessageNode* next;
    MessageNode(string from, string to, string text) {
        fromUser = from;
        toUser = to;
        messageText = text;
        next = nullptr;
        timestamp = getCurrentTime();
    }
};

struct Conversation {
    string friendName;
    MessageNode* msgTop; // stack top
    Conversation* next;
    Conversation(string fname) {
        friendName = fname;
        msgTop = nullptr;
        next = nullptr;
    }
};

struct MsgUser {
    string userName;
    Conversation* convList;
    MessageNode* notificationTop; // stack top
    MsgUser* next;
    MsgUser(string name) {
        userName = name;
        convList = nullptr;
        notificationTop = nullptr;
        next = nullptr;
    }
};

class MessagingSystem {
private:
    MsgUser* userList;

    MsgUser* findMsgUser(string name) {
        MsgUser* temp = userList;
        while (temp) {
            if (temp->userName == name) {
                return temp;
            }
            temp = temp->next;
        }
        return nullptr;
    }

    void addMsgUser(string name) {
        if (findMsgUser(name)) {
            return;
        }
        MsgUser* u = new MsgUser(name);
        u->next = userList;
        userList = u;
    }

    Conversation* getConversation(MsgUser* user, string friendName) {
        Conversation* temp = user->convList;
        while (temp) {
            if (temp->friendName == friendName) {
                return temp;
            }
            temp = temp->next;
        }
        Conversation* conv = new Conversation(friendName);
        conv->next = user->convList;
        user->convList = conv;
        return conv;
    }

public:
    MessagingSystem() { 
        userList = nullptr;
    }

    void sendNotification(string toUser, string text);
    void sendMessage(string fromUser, string toUser, string text);
    void removeUserData(string userName);
    void displayConversation(string user1, string user2);
    void displayNotifications(string user);
    MessageNode* viewLatestMessage(string user, string friendName);
    MessageNode* popLatestMessage(string user, string friendName);
};

// ==========================================
//    MODULE C: ROOMS & SCHEDULING (AVL)
// ==========================================
struct AVLNode {
    string roomID;
    string buildingName;
    int floorNumber;
    string roomType;
    bool isReserved;
    string reservedBy;
    int height;
    AVLNode* left;
    AVLNode* right;

    AVLNode(string id, string bldg, int floor, string type) {
        roomID = id;
        buildingName = bldg;
        floorNumber = floor;
        roomType = type;
        isReserved = false;
        reservedBy = "";
        height = 1;
        left = nullptr;
        right = nullptr;
    }
};

class RoomAVLTree {
private:
    AVLNode* root;

    int getHeight(AVLNode* node) {
        return (node == nullptr) ? 0 : node->height; 
    }
    int maxValue(int a, int b) { 
        return (a > b) ? a : b;
    }
    int getBalanceFactor(AVLNode* node) {
        if (node == nullptr) {
            return 0;
        }
        return getHeight(node->left) - getHeight(node->right);
    }

    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;
        x->right = y;
        y->left = T2;
        y->height = maxValue(getHeight(y->left), getHeight(y->right)) + 1;
        x->height = maxValue(getHeight(x->left), getHeight(x->right)) + 1;
        return x;
    }

    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;
        y->left = x;
        x->right = T2;
        x->height = maxValue(getHeight(x->left), getHeight(x->right)) + 1;
        y->height = maxValue(getHeight(y->left), getHeight(y->right)) + 1;
        return y;
    }

    AVLNode* insertNode(AVLNode* node, string roomID, string bldg, int floor, string type) {
        if (node == nullptr) {
            return new AVLNode(roomID, bldg, floor, type);
        }

        if (roomID < node->roomID) {
            node->left = insertNode(node->left, roomID, bldg, floor, type);
        }
        else if (roomID > node->roomID) {
            node->right = insertNode(node->right, roomID, bldg, floor, type);
        }
        else {
            return node;
        }
        node->height = 1 + maxValue(getHeight(node->left), getHeight(node->right));
        int balance = getBalanceFactor(node);

        // Rotations
        if (balance > 1 && roomID < node->left->roomID) {
            return rightRotate(node);
        }
        if (balance < -1 && roomID > node->right->roomID) {
            return leftRotate(node);
        }
        if (balance > 1 && roomID > node->left->roomID) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && roomID < node->right->roomID) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }

    AVLNode* minValueNode(AVLNode* node) {
        AVLNode* current = node;
        while (current && current->left != nullptr) {
            current = current->left;
        }
        return current;
    }

    AVLNode* deleteNode(AVLNode* node, string roomID) {
        if (node == nullptr) {
            return node;
        }
        if (roomID < node->roomID) {
            node->left = deleteNode(node->left, roomID);
        }
        else if (roomID > node->roomID) {
            node->right = deleteNode(node->right, roomID);
        }
        else {
            if ((node->left == nullptr) || (node->right == nullptr)) {
                AVLNode* temp = node->left ? node->left : node->right;
                if (temp == nullptr) {
                    temp = node;
                    node = nullptr;
                }
                else {
                    *node = *temp;
                }
                delete temp;
            }
            else {
                AVLNode* temp = minValueNode(node->right);
                node->roomID = temp->roomID;
                node->buildingName = temp->buildingName;
                node->floorNumber = temp->floorNumber;
                node->roomType = temp->roomType;
                node->isReserved = temp->isReserved;
                node->reservedBy = temp->reservedBy;
                node->right = deleteNode(node->right, temp->roomID);
            }
        }
        if (node == nullptr) {
            return node;
        }
        node->height = 1 + maxValue(getHeight(node->left), getHeight(node->right));
        return node;
    }

    AVLNode* searchNode(AVLNode* node, string roomID) {
        if (node == nullptr || node->roomID == roomID) {
            return node;
        }
        if (roomID < node->roomID) {
            return searchNode(node->left, roomID);
        }
        return searchNode(node->right, roomID);
    }

    void cancelUserReservations(AVLNode* node, string username, int& count) {
        if (!node) {
            return;
        }
        cancelUserReservations(node->left, username, count);
        if (node->isReserved && node->reservedBy == username) {
            node->isReserved = false;
            node->reservedBy = "";
            count++;
        }
        cancelUserReservations(node->right, username, count);
    }

    void collectRoomsInBuilding(AVLNode* node, string bldg, string* ids, int& count) {
        if (!node) {
            return;
        }
        collectRoomsInBuilding(node->left, bldg, ids, count);
        if (node->buildingName == bldg) {
            ids[count++] = node->roomID;
        }
        collectRoomsInBuilding(node->right, bldg, ids, count);
    }

    void printInorderHelper(AVLNode* node) {
        if (!node) {
            return;
        }
        printInorderHelper(node->left);
        cout << "ID: " << node->roomID << " [" << node->buildingName << "] Floor " << node->floorNumber
            << " Type: " << node->roomType << " -> "
            << (node->isReserved ? ("RESERVED by " + node->reservedBy) : "OPEN") << endl;
        printInorderHelper(node->right);
    }

    void printPreorderHelper(AVLNode* node) {
        if (!node) {
            return;
        }
        cout << "ID: " << node->roomID << " [" << node->buildingName << "]\n";
        printPreorderHelper(node->left);
        printPreorderHelper(node->right);
    }

    void printPostorderHelper(AVLNode* node) {
        if (!node) {
            return;
        }
        printPostorderHelper(node->left);
        printPostorderHelper(node->right);
        cout << "ID: " << node->roomID << " [" << node->buildingName << "]\n";
    }

    int heightHelper(AVLNode* node) {
        if (!node) {
            return 0;
        }
        int lh = heightHelper(node->left);
        int rh = heightHelper(node->right);
        return (lh > rh ? lh : rh) + 1;
    }

    int countLeavesHelper(AVLNode* node) {
        if (!node) {
            return 0;
        }
        if (!node->left && !node->right) {
            return 1;
        }
        return countLeavesHelper(node->left) + countLeavesHelper(node->right);
    }

    int countInternalHelper(AVLNode* node) {
        if (!node) {
            return 0;
        }
        if (!node->left && !node->right) {
            return 0;
        }
        return 1 + countInternalHelper(node->left) + countInternalHelper(node->right);
    }

    void searchRoomsByTypeHelper(AVLNode* node, string type) {
        if (!node) {
            return;
        }
        searchRoomsByTypeHelper(node->left, type);
        if (node->roomType == type) {
            cout << "Room " << node->roomID << " in " << node->buildingName
                << " (Floor " << node->floorNumber << ")\n";
        }
        searchRoomsByTypeHelper(node->right, type);
    }

public:
    RoomAVLTree() { root = nullptr; }

    void insertRoom(string roomID, string bldg, int floor, string type) {
        root = insertNode(root, roomID, bldg, floor, type);
    }

    void deleteRoom(string roomID) {
        root = deleteNode(root, roomID);
    }

    AVLNode* searchRoom(string roomID) {
        return searchNode(root, roomID);
    }

    void searchRoomsByType(string type) {
        cout << "Rooms of type \"" << type << "\":\n";
        searchRoomsByTypeHelper(root, type);
    }

    void cancelAllBy(string username);
    void removeBuildingRooms(string buildingName);
    void reserveRoom(string roomID, string userName);
    void cancelReservation(string roomID);

    bool roomExists(string roomID) { 
        return searchNode(root, roomID) != nullptr; 
    }

    void printInorder() { 
        printInorderHelper(root);
    }
    void printPreorder() { 
        printPreorderHelper(root); 
    }
    void printPostorder() { 
        printPostorderHelper(root); 
    }

    int height() { 
        return heightHelper(root); 
    }
    int countLeaves() {
        return countLeavesHelper(root);
    }
    int countInternalNodes() { 
        return countInternalHelper(root);
    }
};

// ==========================================
//    MODULE D: COMPLAINTS (Queue)
// ==========================================
struct Complaint {
    int ticketID;
    string raisedBy;
    string building;
    string roomID;
    string description;
    string timestamp;
    Complaint* next;
    Complaint(int id, string by, string b, string r, string desc, string time)
        : ticketID(id), raisedBy(by), building(b), roomID(r), description(desc), timestamp(time), next(nullptr) {
    }
};

class ComplaintQueue {
private:
    Complaint* head;
    Complaint* tail;
    int nextTicketID;

public:
    ComplaintQueue() {
        head = tail = nullptr;
        nextTicketID = 1;
    }

    void enqueue(string user, string building, string roomID, string description);

    Complaint* dequeue() {
        if (!head) {
            cout << "No complaints to process.\n";
            return nullptr;
        }
        Complaint* temp = head;
        head = head->next;
        if (!head) {
            tail = nullptr;
        }
        cout << "Processing Ticket #" << temp->ticketID << " by " << temp->raisedBy
            << " - " << temp->description << endl;
        return temp; // caller may delete
    }

    Complaint* peek() {
        if (!head) {
            cout << "No complaints in queue.\n";
            return nullptr;
        }
        cout << "Next Complaint Ticket #" << head->ticketID
            << " by " << head->raisedBy << " - " << head->description << endl;
        return head;
    }

    int countComplaints() {
        int count = 0;
        Complaint* temp = head;
        while (temp) {
            count++;
            temp = temp->next;
        }
        return count;
    }

    void removeByUser(string username) {
        if (!head) {
            return;
        }
        while (head && head->raisedBy == username) {
            Complaint* temp = head;
            head = head->next;
            delete temp;
        }
        if (!head) {
            tail = nullptr;
            return;
        }
        Complaint* curr = head;
        while (curr->next) {
            if (curr->next->raisedBy == username) {
                Complaint* temp = curr->next;
                curr->next = temp->next;
                if (temp == tail) {
                    tail = curr;
                }
                delete temp;
            }
            else {
                curr = curr->next;
            }
        }
    }

    void removeByBuilding(string bldg) {
        if (!head) {
            return;
        }
        while (head && head->building == bldg) {
            Complaint* temp = head;
            head = head->next;
            delete temp;
        }
        if (!head) { 
            tail = nullptr;
            return; 
        }
        Complaint* curr = head;
        while (curr->next) {
            if (curr->next->building == bldg) {
                Complaint* temp = curr->next;
                curr->next = temp->next;
                if (temp == tail) tail = curr;
                delete temp;
            }
            else {
                curr = curr->next;
            }
        }
    }

    void displayQueue() {
        Complaint* temp = head;
        if (!temp) {
            cout << "No complaints pending.\n";
            return;
        }
        while (temp) {
            cout << "Ticket #" << temp->ticketID << ": " << temp->description
                << " (By: " << temp->raisedBy << ", Building: " << temp->building << ")\n";
            temp = temp->next;
        }
    }
};

// ==========================================
//    MODULE A: PEOPLE DIRECTORY (Hash)
// ==========================================
struct HashEntry {
    string userName;
    string password;
    string role;
    string department;
    string email;
    string lastBookedRoom;
    HashEntry* next;
};

class PeopleDirectory {
    HashEntry** table;
    int capacity;
    int size;

public:
    PeopleDirectory(int cap = 10) {
        capacity = cap;
        size = 0;
        table = new HashEntry * [capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = nullptr;
        }
    }

    int hashFunc(const string& username) {
        int sum = 0;
        for (size_t i = 0; i < username.length(); i++) {
            sum += (int)username[i];
        }
        return sum % capacity;
    }

    bool userExists(string username) {
        int index = hashFunc(username);
        HashEntry* current = table[index];
        while (current) {
            if (current->userName == username) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    void insert(string username, string password, string role, string department, string email) {
        if (userExists(username)) { 
            cout << "User already exists!\n"; 
            return;
        }
        int index = hashFunc(username);
        HashEntry* newNode = new HashEntry{ username, password, role, department, email, "None", nullptr };
        newNode->next = table[index];
        table[index] = newNode;
        size++;
        cout << "User Registered: " << username << endl;
    }

    bool login(string username, string password) {
        int index = hashFunc(username);
        HashEntry* current = table[index];
        while (current) {
            if (current->userName == username && current->password == password) {
                return true;
            }
            current = current->next;
        }
        return false;
    }

    void updateLastBookedRoom(string username, string room) {
        int index = hashFunc(username);
        HashEntry* current = table[index];
        while (current) {
            if (current->userName == username) {
                current->lastBookedRoom = room;
                return;
            }
            current = current->next;
        }
    }

    void updateProfile(string username, string newDept, string newEmail) {
        int index = hashFunc(username);
        HashEntry* current = table[index];
        while (current) {
            if (current->userName == username) {
                current->department = newDept;
                current->email = newEmail;
                cout << "Profile updated for " << username << endl;
                return;
            }
            current = current->next;
        }
        cout << "User not found.\n";
    }

    void showUser(string username) {
        int index = hashFunc(username);
        HashEntry* current = table[index];
        while (current) {
            if (current->userName == username) {
                cout << "User: " << current->userName
                    << " | Role: " << current->role
                    << " | Dept: " << current->department
                    << " | Email: " << current->email
                    << " | Last Room: " << current->lastBookedRoom << endl;
                return;
            }
            current = current->next;
        }
        cout << "User not found.\n";
    }

    void deleteUser(string username);

    void displayAll() {
        for (int i = 0; i < capacity; i++) {
            HashEntry* curr = table[i];
            while (curr) {
                cout << "User: " << curr->userName
                    << " | Role: " << curr->role
                    << " | Dept: " << curr->department
                    << " | Email: " << curr->email
                    << " | Last Room: " << curr->lastBookedRoom << endl;
                curr = curr->next;
            }
        }
    }
};

// ==========================================
//    MODULE B: CAMPUS MAP (Graph)
// ==========================================
class CampusMap {
    struct Edge {
        string destBuildingName;
        string relation;
        string status; // open/closed (optional)
        int distance;
        Edge* next;
        Edge(string dest, string rel, string st, int dist)
            : destBuildingName(dest), relation(rel), status(st), distance(dist), next(nullptr) {
        }
    };

    struct Building {
        string buildingName;
        Edge* head;
        Building(string name) : buildingName(name), head(nullptr) {}
    };

    Building** buildings;
    int capacity;
    int buildingCount;

public:
    CampusMap(int cap = 20) {
        capacity = cap;
        buildingCount = 0;
        buildings = new Building * [capacity];
        for (int i = 0; i < capacity; i++) {
            buildings[i] = nullptr;
        }
    }

    int findBuildingIndex(const string& name) {
        for (int i = 0; i < buildingCount; i++) {
            if (buildings[i]->buildingName == name) {
                return i;
            }
        }
        return -1;
    }

    bool buildingExists(string name) { 
        return findBuildingIndex(name) != -1; 
    }

    void addBuilding(const string& name) {
        if (buildingCount >= capacity) { 
            cout << "Capacity full.\n";
            return;
        }
        if (findBuildingIndex(name) != -1) { 
            cout << "Building already exists.\n";
            return;
        }
        buildings[buildingCount++] = new Building(name);
        cout << "Building added: " << name << endl;
    }

    void addPath(string src, string dest, string rel, int dist, string status = "open") {
        int u = findBuildingIndex(src);
        int v = findBuildingIndex(dest);
        if (u == -1 || v == -1) {
            cout << "One or both buildings not found.\n";
            return;
        }

        Edge* e1 = new Edge(dest, rel, status, dist);
        e1->next = buildings[u]->head;
        buildings[u]->head = e1;

        Edge* e2 = new Edge(src, rel, status, dist);
        e2->next = buildings[v]->head;
        buildings[v]->head = e2;

        cout << "Path added between " << src << " and " << dest << endl;
    }

    void removePath(const string& src, const string& dest) {
        int u = findBuildingIndex(src);
        int v = findBuildingIndex(dest);
        if (u == -1 || v == -1) {
            cout << "Buildings not found.\n";
            return;
        }

        // remove dest from src list
        Edge* curr = buildings[u]->head;
        Edge* prev = nullptr;
        while (curr) {
            if (curr->destBuildingName == dest) {
                if (prev) {
                    prev->next = curr->next;
                }
                else {
                    buildings[u]->head = curr->next;
                }
                delete curr;
                break;
            }
            prev = curr;
            curr = curr->next;
        }

        // remove src from dest list
        curr = buildings[v]->head;
        prev = nullptr;
        while (curr) {
            if (curr->destBuildingName == src) {
                if (prev) {
                    prev->next = curr->next;
                }
                else {
                    buildings[v]->head = curr->next;
                }
                delete curr;
                break;
            }
            prev = curr;
            curr = curr->next;
        }

        cout << "Path removed between " << src << " and " << dest << endl;
    }

    bool isConnected(const string& src, const string& dest) {
        int s = findBuildingIndex(src);
        int d = findBuildingIndex(dest);
        if (s == -1 || d == -1) {
            cout << "Invalid buildings.\n";
            return false;
        }

        bool* visited = new bool[capacity];
        for (int i = 0; i < capacity; i++) {
            visited[i] = false;
        }
        MapQueue q;
        visited[s] = true;
        q.enqueue(s);

        while (!q.isEmpty()) {
            int u = q.dequeue();
            if (u == d) {
                delete[] visited;
                return true;
            }
            Edge* temp = buildings[u]->head;
            while (temp) {
                int v = findBuildingIndex(temp->destBuildingName);
                if (v != -1 && !visited[v]) {
                    visited[v] = true;
                    q.enqueue(v);
                }
                temp = temp->next;
            }
        }
        delete[] visited;
        return false;
    }

    void removeBuilding(string name);

    void BFS(string startBuilding) {
        int startIdx = findBuildingIndex(startBuilding);
        if (startIdx == -1) {
            cout << "Start building not found.\n";
            return;
        }

        bool* visited = new bool[capacity];
        for (int i = 0; i < capacity; i++) {
            visited[i] = false;
        }

        MapQueue q;
        visited[startIdx] = true;
        q.enqueue(startIdx);

        cout << "BFS Traversal: ";
        while (!q.isEmpty()) {
            int u = q.dequeue();
            cout << buildings[u]->buildingName << " -> ";

            Edge* temp = buildings[u]->head;
            while (temp) {
                int v = findBuildingIndex(temp->destBuildingName);
                if (v != -1 && !visited[v]) {
                    visited[v] = true;
                    q.enqueue(v);
                }
                temp = temp->next;
            }
        }
        cout << "End\n";
        delete[] visited;
    }

    void DFS(string startBuilding) {
        int startIdx = findBuildingIndex(startBuilding);
        if (startIdx == -1) {
            cout << "Start building not found.\n";
            return;
        }

        bool* visited = new bool[capacity];
        for (int i = 0; i < capacity; i++) {
            visited[i] = false;
        }

        MapStack s;
        s.push(startIdx);

        cout << "DFS Traversal: ";
        while (!s.isEmpty()) {
            int u = s.pop();
            if (!visited[u]) {
                visited[u] = true;
                cout << buildings[u]->buildingName << " -> ";
                Edge* temp = buildings[u]->head;
                while (temp) {
                    int v = findBuildingIndex(temp->destBuildingName);
                    if (v != -1 && !visited[v]) {
                        s.push(v);
                    }
                    temp = temp->next;
                }
            }
        }
        cout << "End\n";
        delete[] visited;
    }

    void shortestPath(const string& start, const string& end) {
        int startIndex = findBuildingIndex(start);
        int endIndex = findBuildingIndex(end);
        if (startIndex == -1 || endIndex == -1) {
            cout << "Invalid buildings.\n";
            return;
        }

        int* dist = new int[capacity];
        bool* visited = new bool[capacity];
        int* prev = new int[capacity];

        for (int i = 0; i < capacity; i++) {
            dist[i] = INT_MAX;
            visited[i] = false; 
            prev[i] = -1;
        }
        dist[startIndex] = 0;

        for (int i = 0; i < buildingCount; i++) {
            int u = -1;
            for (int j = 0; j < buildingCount; j++) {
                if (!visited[j] && (u == -1 || dist[j] < dist[u])) {
                    u = j;
                }
            }
            if (u == -1 || dist[u] == INT_MAX || u == endIndex) {
                break;
            }
            visited[u] = true;

            Edge* curr = buildings[u]->head;
            while (curr) {
                int v = findBuildingIndex(curr->destBuildingName);
                if (v != -1 && !visited[v]) {
                    int alt = dist[u] + curr->distance;
                    if (alt < dist[v]) {
                        dist[v] = alt;
                        prev[v] = u;
                    }
                }
                curr = curr->next;
            }
        }

        if (dist[endIndex] == INT_MAX) {
            cout << "No path found.\n";
        }
        else {
            cout << "Shortest Distance: " << dist[endIndex] << "m\nPath: ";
            printPath(endIndex, prev);
            cout << endl;
        }
        delete[] dist;
        delete[] visited;
        delete[] prev;
    }

    void printPath(int curr, int* prev) {
        if (curr == -1) {
            return;
        }
        printPath(prev[curr], prev);
        cout << buildings[curr]->buildingName << (prev[curr] != -1 ? " -> " : "");
    }

    void displayMap() {
        for (int i = 0; i < buildingCount; i++) {
            cout << buildings[i]->buildingName << " connects to: ";
            Edge* temp = buildings[i]->head;
            while (temp) {
                cout << temp->destBuildingName << " (" << temp->distance << "m) ";
                temp = temp->next;
            }
            cout << endl;
        }
    }
};

// ==========================================
//    GLOBAL DEFINITIONS
// ==========================================
PeopleDirectory g_people;
CampusMap g_map;
RoomAVLTree g_rooms;
ComplaintQueue g_complaints;
MessagingSystem g_messaging;

// ==========================================
//    IMPLEMENTATIONS (Integration Logic)
// ==========================================

// --- Messaging System ---
void MessagingSystem::sendNotification(string toUser, string text) {
    if (!g_people.userExists(toUser)) {
        return;
    }
    addMsgUser(toUser);
    MsgUser* u = findMsgUser(toUser);
    MessageNode* note = new MessageNode("System", toUser, text);
    note->next = u->notificationTop;
    u->notificationTop = note;
}

void MessagingSystem::sendMessage(string fromUser, string toUser, string text) {
    if (!g_people.userExists(fromUser) || !g_people.userExists(toUser)) {
        cout << "Error: One or both users do not exist.\n";
        return;
    }
    addMsgUser(fromUser);
    addMsgUser(toUser);
    MsgUser* sender = findMsgUser(fromUser);
    MsgUser* receiver = findMsgUser(toUser);
    Conversation* senderConv = getConversation(sender, toUser);
    Conversation* receiverConv = getConversation(receiver, fromUser);

    MessageNode* msg1 = new MessageNode(fromUser, toUser, text);
    msg1->next = senderConv->msgTop;
    senderConv->msgTop = msg1;

    MessageNode* msg2 = new MessageNode(fromUser, toUser, text);
    msg2->next = receiverConv->msgTop;
    receiverConv->msgTop = msg2;

    sendNotification(toUser, "New message from " + fromUser);
    cout << "Message sent!\n";
}

void MessagingSystem::removeUserData(string userName) {
    MsgUser* prev = nullptr;
    MsgUser* curr = userList;
    while (curr) {
        if (curr->userName == userName) {
            if (prev) {
                prev->next = curr->next;
            }
            else {
                userList = curr->next;
            }

            // free conversations & messages & notifications
            Conversation* c = curr->convList;
            while (c) {
                MessageNode* m = c->msgTop;
                while (m) {
                    MessageNode* tmpm = m;
                    m = m->next;
                    delete tmpm;
                }
                Conversation* tmpc = c;
                c = c->next;
                delete tmpc;
            }
            MessageNode* n = curr->notificationTop;
            while (n) {
                MessageNode* tmpn = n;
                n = n->next;
                delete tmpn;
            }

            delete curr;
            cout << "Messaging data cleared for " << userName << endl;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

void MessagingSystem::displayConversation(string user1, string user2) {
    MsgUser* u = findMsgUser(user1);
    if (!u) { 
        cout << "No messages.\n";
        return;
    }
    Conversation* c = getConversation(u, user2);
    MessageNode* t = c->msgTop;
    if (!t) { 
        cout << "No messages in this conversation.\n";
        return; 
    }
    cout << "--- Chat with " << user2 << " ---\n";
    while (t) {
        cout << "[" << t->timestamp << "] " << (t->fromUser == user1 ? "Me" : user2)
            << ": " << t->messageText << endl;
        t = t->next;
    }
}

void MessagingSystem::displayNotifications(string user) {
    MsgUser* u = findMsgUser(user);
    if (!u || !u->notificationTop) { 
        cout << "No notifications.\n";
        return;
    }
    cout << "--- Notifications ---\n";
    MessageNode* t = u->notificationTop;
    while (t) {
        cout << t->messageText << " (" << t->timestamp << ")\n";
        t = t->next;
    }
}

MessageNode* MessagingSystem::viewLatestMessage(string user, string friendName) {
    MsgUser* u = findMsgUser(user);
    if (!u) { 
        cout << "No messages.\n";
        return nullptr; 
    }
    Conversation* c = getConversation(u, friendName);
    if (!c->msgTop) {
        cout << "No messages with " << friendName << ".\n"; 
        return nullptr;
    }
    MessageNode* top = c->msgTop;
    cout << "Latest message with " << friendName << ":\n";
    cout << "[" << top->timestamp << "] " << top->fromUser << ": " << top->messageText << endl;
    return top;
}

MessageNode* MessagingSystem::popLatestMessage(string user, string friendName) {
    MsgUser* u = findMsgUser(user);
    if (!u) { 
        cout << "No messages.\n";
        return nullptr;
    }
    Conversation* c = getConversation(u, friendName);
    if (!c->msgTop) { 
        cout << "No messages to pop.\n";
        return nullptr; 
    }
    MessageNode* top = c->msgTop;
    c->msgTop = top->next;
    cout << "Popped message: [" << top->timestamp << "] " << top->fromUser << ": " << top->messageText << endl;
    return top; // caller may delete or ignore
}

// --- People Directory ---
void PeopleDirectory::deleteUser(string username) {
    int index = hashFunc(username);
    HashEntry* current = table[index];
    HashEntry* prev = nullptr;

    while (current) {
        if (current->userName == username) {
            cout << "Deleting User " << username << "...\n";

            // Integration: cancel room reservations, clear messaging, remove complaints
            g_rooms.cancelAllBy(username);
            g_messaging.removeUserData(username);
            g_complaints.removeByUser(username);

            if (prev) {
                prev->next = current->next;
            }
            else {
                table[index] = current->next;
            }
            delete current;
            size--;
            cout << "User deleted successfully.\n";
            return;
        }
        prev = current;
        current = current->next;
    }
    cout << "User not found.\n";
}

// --- Campus Map ---
void CampusMap::removeBuilding(string name) {
    int index = findBuildingIndex(name);
    if (index == -1) {
        cout << "Building not found.\n";
        return;
    }

    // remove all edges to this building
    for (int i = 0; i < buildingCount; i++) {
        if (i == index) {
            continue;
        }
        Edge* curr = buildings[i]->head;
        Edge* prev = nullptr;
        while (curr) {
            if (curr->destBuildingName == name) {
                if (prev) {
                    prev->next = curr->next;
                }
                else {
                    buildings[i]->head = curr->next;
                }
                delete curr;
                break;
            }
            prev = curr;
            curr = curr->next;
        }
    }

    // integration: remove rooms and complaints of this building
    g_rooms.removeBuildingRooms(name);
    g_complaints.removeByBuilding(name);

    // delete adjacency list for this building
    Edge* e = buildings[index]->head;
    while (e) {
        Edge* tmp = e;
        e = e->next;
        delete tmp;
    }

    delete buildings[index];
    for (int i = index; i < buildingCount - 1; i++) {
        buildings[i] = buildings[i + 1];
    }
    buildingCount--;

    cout << "Building " << name << " and all associated data removed.\n";
}

// --- Room AVL ---
void RoomAVLTree::cancelAllBy(string username) {
    int count = 0;
    cancelUserReservations(root, username, count);
    if (count > 0) {
        cout << "Cancelled " << count << " reservations for " << username << endl;
    }
}

void RoomAVLTree::removeBuildingRooms(string buildingName) {
    string ids[500];
    int count = 0;
    collectRoomsInBuilding(root, buildingName, ids, count);
    for (int i = 0; i < count; i++) {
        root = deleteNode(root, ids[i]);
    }
    cout << "Removed " << count << " rooms for building " << buildingName << endl;
}

void RoomAVLTree::reserveRoom(string roomID, string userName) {
    if (!g_people.userExists(userName)) {
        cout << "Error: User does not exist.\n";
        return;
    }
    AVLNode* node = searchNode(root, roomID);
    if (!node) {
        cout << "Room not found.\n";
        return;
    }
    if (node->isReserved) {
        cout << "Error: Room already reserved by " << node->reservedBy << endl;
        return;
    }
    node->isReserved = true;
    node->reservedBy = userName;
    g_people.updateLastBookedRoom(userName, roomID);
    g_messaging.sendNotification(userName, "You reserved room " + roomID);
    cout << "Reservation Successful for " << userName << endl;
}

void RoomAVLTree::cancelReservation(string roomID) {
    AVLNode* node = searchNode(root, roomID);
    if (node && node->isReserved) {
        string user = node->reservedBy;
        node->isReserved = false;
        node->reservedBy = "";
        g_messaging.sendNotification(user, "Your reservation for " + roomID + " was cancelled.");
        cout << "Reservation cancelled.\n";
    }
    else {
        cout << "Room not reserved or not found.\n";
    }
}

// --- Complaint Queue ---
void ComplaintQueue::enqueue(string user, string building, string roomID, string description) {
    if (!g_people.userExists(user)) {
        cout << "Error: User not found.\n";
        return;
    }
    if (!g_map.buildingExists(building)) {
        cout << "Error: Building not found.\n";
        return;
    }
    Complaint* newNode = new Complaint(nextTicketID++, user, building, roomID, description, getCurrentTime());
    if (!head) {
        head = tail = newNode;
    }
    else {
        tail->next = newNode;
        tail = newNode;
    }
    cout << "Complaint Ticket #" << newNode->ticketID << " submitted.\n";
}

// ==========================================
//              MAIN MENU
// ==========================================
int main() {
    cout << " ====== UNIVERSITY MANAGEMENT SYSTEM ======\n" << endl;
    // Seed some data
    g_people.insert("admin", "123", "admin", "Administration", "admin@nu.edu.pk");
    g_people.insert("ali", "ali123", "student", "CS", "ali@nu.edu.pk");
    g_people.insert("sara", "sara123", "faculty", "CS", "sara@nu.edu.pk");
    cout << endl;

    g_map.addBuilding("CS-Block");
    g_map.addBuilding("Main-Hall");
    g_map.addBuilding("Library");
    cout << endl;

    g_map.addPath("CS-Block", "Main-Hall", "Walkway", 50);
    g_map.addPath("Main-Hall", "Library", "Road", 120);
    g_map.addPath("CS-Block", "Library", "Shortcut", 180);

    g_rooms.insertRoom("CS-101", "CS-Block", 1, "Lab");
    g_rooms.insertRoom("CS-102", "CS-Block", 1, "Classroom");
    g_rooms.insertRoom("MH-01", "Main-Hall", 0, "Auditorium");
    g_rooms.insertRoom("LIB-201", "Library", 2, "StudyRoom");
    

    int mainChoice;
    string selectedBuilding = "";

    do {
        cout << "\n================ MAIN MENU =================\n";
        cout << "1. User & Authentication (Hash Table)\n";
        cout << "2. Campus Map (Graph)\n";
        cout << "3. Rooms & Scheduling (AVL)\n";
        cout << "4. Complaints (Queue)\n";
        cout << "5. Messaging (Stacks)\n";
        cout << "6. Exit\n";
        cout << "============================================\n";
        cout << "Enter your choice: ";
        cin >> mainChoice;
        cin.ignore();

        // ---------------- USER & AUTH ----------------
        if (mainChoice == 1) {
            int sub;
            do {
                cout << "\n--- User Directory Menu ---\n";
                cout << "1. Register New User\n";
                cout << "2. Login\n";
                cout << "3. Update Profile\n";
                cout << "4. Delete User\n";
                cout << "5. Search User\n";
                cout << "6. Display All Users (Buckets)\n";
                cout << "7. Back\n";
                cout << "Enter your choice: ";
                cin >> sub;
                cin.ignore();

                if (sub == 1) {
                    string u, p, r, d, e;
                    cout << "Username: ";
                    getline(cin, u);
                    cout << "Password: ";
                    getline(cin, p);
                    cout << "Role (student/teacher/admin): ";
                    getline(cin, r);
                    cout << "Department: ";
                    getline(cin, d);
                    cout << "Email: ";
                    getline(cin, e);
                    g_people.insert(u, p, r, d, e);
                }
                else if (sub == 2) {
                    string u, p;
                    cout << "Username: ";
                    getline(cin, u);
                    cout << "Password: "; 
                    getline(cin, p);
                    if (g_people.login(u, p)) cout << "Login successful.\n";
                    else cout << "Invalid username or password.\n";
                }
                else if (sub == 3) {
                    string u, d, e;
                    cout << "Username: ";
                    getline(cin, u);
                    cout << "New Department: ";
                    getline(cin, d);
                    cout << "New Email: "; 
                    getline(cin, e);
                    g_people.updateProfile(u, d, e);
                }
                else if (sub == 4) {
                    string u;
                    cout << "Username to delete: "; 
                    getline(cin, u);
                    g_people.deleteUser(u);
                }
                else if (sub == 5) {
                    string u;
                    cout << "Username to search: ";
                    getline(cin, u);
                    if (g_people.userExists(u)) {
                        cout << "User exists.\n";
                        g_people.showUser(u);
                    }
                    else {
                        cout << "User not found.\n";
                    }
                }
                else if (sub == 6) {
                    g_people.displayAll();
                }

            } while (sub != 7);
        }

        // ---------------- CAMPUS MAP ----------------
        else if (mainChoice == 2) {
            int sub;
            do {
                cout << "\n--- Campus Map Menu ---\n";
                cout << "1. Add Building\n";
                cout << "2. Remove Building\n";
                cout << "3. Add Path between Buildings\n";
                cout << "4. Remove Path\n";
                cout << "5. Find Shortest Path (Dijkstra)\n";
                cout << "6. BFS from Building\n";
                cout << "7. DFS from Building\n";
                cout << "8. Display Adjacency List\n";
                cout << "9. Back\n";
                cout << "Enter your choice: ";
                cin >> sub;
                cin.ignore();

                if (sub == 1) {
                    string b;
                    cout << "Building Name: ";
                    getline(cin, b);
                    g_map.addBuilding(b);
                }
                else if (sub == 2) {
                    string b;
                    cout << "Building Name to remove: ";
                    getline(cin, b);
                    g_map.removeBuilding(b);
                }
                else if (sub == 3) {
                    string s, d, r; int dist;
                    cout << "From Building: ";
                    getline(cin, s);
                    cout << "To Building: "; 
                    getline(cin, d);
                    cout << "Relation (road/walkway): ";
                    getline(cin, r);
                    cout << "Distance (m): "; 
                    cin >> dist; 
                    cin.ignore();
                    g_map.addPath(s, d, r, dist);
                }
                else if (sub == 4) {
                    string s, d;
                    cout << "From Building: ";
                    getline(cin, s);
                    cout << "To Building: ";
                    getline(cin, d);
                    g_map.removePath(s, d);
                }
                else if (sub == 5) {
                    string s, d;
                    cout << "Start Building: ";
                    getline(cin, s);
                    cout << "End Building: ";
                    getline(cin, d);
                    g_map.shortestPath(s, d);
                    cout << (g_map.isConnected(s, d) ? "They are connected.\n" : "They are NOT connected.\n");
                }
                else if (sub == 6) {
                    string s;
                    cout << "Start Building: "; 
                    getline(cin, s);
                    g_map.BFS(s);
                }
                else if (sub == 7) {
                    string s;
                    cout << "Start Building: ";
                    getline(cin, s);
                    g_map.DFS(s);
                }
                else if (sub == 8) {
                    g_map.displayMap();
                }

            } while (sub != 9);
        }

        // ---------------- ROOMS & AVL ----------------
        else if (mainChoice == 3) {
            int sub;
            do {
                cout << "\n--- Rooms & AVL Menu ---\n";
                cout << "1. Select Building (for reference only)\n";
                cout << "2. Insert Room\n";
                cout << "3. Delete Room\n";
                cout << "4. Search Room\n";
                cout << "5. Search Rooms by Type\n";
                cout << "6. Reserve Room\n";
                cout << "7. Cancel Reservation\n";
                cout << "8. Print Inorder Traversal\n";
                cout << "9. Print Preorder Traversal\n";
                cout << "10. Print Postorder Traversal\n";
                cout << "11. Back\n";
                cout << "Enter your choice: ";
                cin >> sub;
                cin.ignore();

                if (sub == 1) {
                    cout << "Enter Building Name to select: ";
                    getline(cin, selectedBuilding);
                    cout << "Selected building: " << selectedBuilding << endl;
                }
                else if (sub == 2) {
                    string id, b, t; int f;
                    if (selectedBuilding != "") {
                        b = selectedBuilding;
                        cout << "Using selected building: " << b << endl;
                    }
                    else {
                        cout << "Building Name: "; getline(cin, b);
                    }
                    cout << "Room ID: "; 
                    getline(cin, id);
                    cout << "Floor Number: ";
                    cin >> f;
                    cin.ignore();
                    cout << "Room Type (classroom/lab/office/etc.): ";
                    getline(cin, t);
                    g_rooms.insertRoom(id, b, f, t);
                }
                else if (sub == 3) {
                    string id;
                    cout << "Room ID to delete: ";
                    getline(cin, id);
                    g_rooms.deleteRoom(id);
                }
                else if (sub == 4) {
                    string id;
                    cout << "Room ID to search: "; 
                    getline(cin, id);
                    AVLNode* n = g_rooms.searchRoom(id);
                    if (n) {
                        cout << "Found Room " << n->roomID << " in " << n->buildingName
                            << " Floor " << n->floorNumber << " Type " << n->roomType
                            << " -> " << (n->isReserved ? ("RESERVED by " + n->reservedBy) : "OPEN") << endl;
                    }
                    else {
                        cout << "Room not found.\n";
                    }
                }
                else if (sub == 5) {
                    string t;
                    cout << "Room Type to search: ";
                    getline(cin, t);
                    g_rooms.searchRoomsByType(t);
                }
                else if (sub == 6) {
                    string id, u;
                    cout << "Room ID: ";
                    getline(cin, id);
                    cout << "Username: ";
                    getline(cin, u);
                    g_rooms.reserveRoom(id, u);
                }
                else if (sub == 7) {
                    string id;
                    cout << "Room ID: ";
                    getline(cin, id);
                    g_rooms.cancelReservation(id);
                }
                else if (sub == 8) {
                    g_rooms.printInorder();
                }
                else if (sub == 9) {
                    g_rooms.printPreorder();
                }
                else if (sub == 10) {
                    g_rooms.printPostorder();
                }

            } while (sub != 11);
        }

        // ---------------- COMPLAINTS ----------------
        else if (mainChoice == 4) {
            int sub;
            do {
                cout << "\n--- Complaints Menu ---\n";
                cout << "1. Submit New Complaint\n";
                cout << "2. Process Next Complaint (Dequeue)\n";
                cout << "3. View Next Complaint (Peek)\n";
                cout << "4. Display All Pending\n";
                cout << "5. Complaint Statistics\n";
                cout << "6. Back\n";
                cout << "Enter your choice: ";
                cin >> sub;
                cin.ignore();

                if (sub == 1) {
                    string u, b, r, d;
                    cout << "Your Username: ";
                    getline(cin, u);
                    cout << "Building: ";
                    getline(cin, b);
                    cout << "Room ID: ";
                    getline(cin, r);
                    cout << "Description: ";
                    getline(cin, d);
                    g_complaints.enqueue(u, b, r, d);
                }
                else if (sub == 2) {
                    Complaint* c = g_complaints.dequeue();
                    if (c) {
                        delete c;
                    }
                }
                else if (sub == 3) {
                    g_complaints.peek();
                }
                else if (sub == 4) {
                    g_complaints.displayQueue();
                }
                else if (sub == 5) {
                    cout << "Total pending complaints: " << g_complaints.countComplaints() << endl;
                }

            } while (sub != 6);
        }

        // ---------------- MESSAGING ----------------
        else if (mainChoice == 5) {
            int sub;
            do {
                cout << "\n--- Messaging System Menu ---\n";
                cout << "1. Send Message to User\n";
                cout << "2. View Latest Message in Conversation\n";
                cout << "3. Pop Latest Message\n";
                cout << "4. Display Full Conversation\n";
                cout << "5. View My Notifications\n";
                cout << "6. Back\n";
                cout << "Enter your choice: ";
                cin >> sub;
                cout << endl;
                cin.ignore();

                if (sub == 1) {
                    string f, t, m;
                    cout << "From (your username): "; 
                    getline(cin, f);
                    cout << "To (friend username): ";
                    getline(cin, t);
                    cout << "Message: ";
                    getline(cin, m);
                    g_messaging.sendMessage(f, t, m);
                }
                else if (sub == 2) {
                    string u, f;
                    cout << "Your username: ";
                    getline(cin, u);
                    cout << "Friend username: ";
                    getline(cin, f);
                    g_messaging.viewLatestMessage(u, f);
                }
                else if (sub == 3) {
                    string u, f;
                    cout << "Your username: ";
                    getline(cin, u);
                    cout << "Friend username: ";
                    getline(cin, f);
                    MessageNode* msg = g_messaging.popLatestMessage(u, f);
                    if (msg) {
                        delete msg;
                    }
                }
                else if (sub == 4) {
                    string u, f;
                    cout << "Your username: ";
                    getline(cin, u);
                    cout << "Friend username: ";
                    getline(cin, f);
                    g_messaging.displayConversation(u, f);
                }
                else if (sub == 5) {
                    string u;
                    cout << "Your username: ";
                    getline(cin, u);
                    g_messaging.displayNotifications(u);
                }

            } while (sub != 6);
        }

        else if (mainChoice != 6) {
            cout << "Invalid choice.\n";
        }

    } while (mainChoice != 6);

    cout << "Exiting University Management System...\n";
    system("pause>0");
    return 0;
}