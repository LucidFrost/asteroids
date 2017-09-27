#if AS_HEADER

struct Powerup {
    Entity* entity = null;
};

void on_create(Powerup* powerup);
void on_destroy(Powerup* powerup);
void on_update(Powerup* powerup);
void on_collision(Powerup* us, Entity* them);

#else

void on_create(Powerup* powerup) {

}

void on_destroy(Powerup* powerup) {

}

void on_update(Powerup* powerup) {

}

void on_collision(Powerup* us, Entity* them) {

}

#endif