# Proteger uma folha ou o livro
<!-- keywords: proteger, proteção, palavra-passe, senha, bloquear, células bloqueadas, desbloquear, desproteger, estrutura, folhas, só de leitura, permitir, formatar células, inserir linhas, ordenar, filtrar, hash -->

## Proteger uma folha

**Rever > Proteger Folha** impede que a folha seja alterada por engano. Numa folha protegida, as células que estão **bloqueadas** não podem ser alteradas. Todas as células estão bloqueadas até indicar o contrário: selecione as células que podem ser alteradas, abra **Formatar Células > Proteção** e desmarque **Bloqueada**. (Isto só tem efeito depois de a folha estar protegida.)

A caixa de diálogo pergunta o que todos podem continuar a fazer na folha: selecionar células bloqueadas ou desbloqueadas, formatar células, alterar colunas ou linhas, inserir ou eliminar linhas e colunas, inserir ligações, ordenar, utilizar filtros e alterar objetos de desenho e notas. Tudo o que deixar desmarcado é recusado, com uma mensagem que indica o que foi recusado.

A **palavra-passe** é opcional. Se definir uma, a folha só pode ser desprotegida com ela. O JeffCrunch guarda uma forma codificada da palavra-passe (um hash com salt, repetido 100 000 vezes) e nunca a própria palavra-passe, pelo que não a pode mostrar mais tarde: guarde-a num local seguro. As palavras-passe criadas por outros programas, tanto no formato mais antigo como no mais recente, são verificadas da mesma forma.

Proteger e desproteger são passos que pode anular. Para retirar a proteção, utilize **Desproteger Folha** e escreva a palavra-passe, se existir.

Numa folha protegida, algumas coisas continuam proibidas, seja o que for que permita: unir células e eliminar linhas ou colunas que contenham células bloqueadas.

## Proteger o livro

**Rever > Proteger Livro** bloqueia a **estrutura**: não é possível adicionar, eliminar, mudar o nome, mover, ocultar ou mostrar nenhuma folha. Também pode pedir que as janelas sejam mantidas como estão. Pode igualmente ter uma palavra-passe. **Desproteger Livro** retira a proteção.
