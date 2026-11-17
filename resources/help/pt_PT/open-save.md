# Guardar e abrir publicações
<!-- keywords: abrir, guardar, guardar como, guardar uma cópia, fechar, ficheiros recentes, afixados, tipos de ficheiro, jpub, renomear, cópia de segurança, propriedades, informações, marcar como final, inspecionar, histórico de versões, novo nome, copiar, Ctrl+S, só de leitura, apenas leitura, abrir como cópia, nova janela, os meus modelos, guardar como modelo, reparar, abrir e reparar, danificado, verificar compatibilidade, guardar mesmo assim, não voltar a perguntar -->

O JeffPub guarda o seu trabalho em ficheiros .jpub. Também abre e guarda ficheiros .pub. Consulte [Trabalhar com ficheiros .pub](pub-files).

## Guardar

Prima Ctrl+S, clique no botão **Guardar** na barra de ferramentas rápida ou clique em **Ficheiro > Guardar**. Da primeira vez, o JeffPub pede um nome e um local. Depois disso, Guardar substitui o ficheiro pelo seu trabalho mais recente.

Um asterisco (*) depois do nome na barra de título indica que existem alterações não guardadas. Guarde com frequência. Se o JeffPub parar inesperadamente, consulte [Recuperar trabalho não guardado](recover).

Para manter a última cópia guardada de um ficheiro, ative **Criar sempre cópia de segurança**. Sempre que guardar, o JeffPub copia primeiro o ficheiro antigo para outro com o nome "Backup of" seguido do nome do ficheiro, na mesma pasta. Se não for possível criar a cópia, a aplicação pergunta se deve guardar sem ela. A resposta predefinida é Não, a menos que responda Sim, e o ficheiro mantém-se como estava. Esta opção encontra-se em **Ficheiro > Opções**, no separador **Geral**. Todas as aplicações JeffOffice a têm.

## Guardar com um novo nome

Clique em **Ficheiro > Guardar Como** ou prima F12. Escreva um nome, escolha **Guardar como tipo** e clique em **Guardar**. As opções são:

- **Publicação JeffPub (*.jpub)** é a escolha normal.
- **Ficheiros de publicação .pub (*.pub)** cria um ficheiro .pub.
- **PDF (*.pdf)** cria um PDF e deixa a sua publicação como estava.
- **Modelo JeffPub (*.jpub)** acrescenta a publicação a **Os Meus Modelos**.

Depois de guardar como .jpub ou .pub, a barra de título mostra o novo nome, e continua a trabalhar no novo ficheiro.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia** e escolha um nome. O JeffPub escreve a publicação nesse ficheiro, e continua a trabalhar no que tinha aberto. O nome e as alterações não guardadas do ficheiro atual mantêm-se como estavam.

## Abrir

1. Prima Ctrl+O ou clique em **Ficheiro > Abrir**.
2. Clique em **Procurar** para escolher um ficheiro. Ou clique num nome em **Afixados** ou **Publicações Recentes**.

Também pode arrastar um ficheiro .jpub ou .pub para a página. Se a publicação atual tiver alterações não guardadas, o JeffPub pergunta primeiro sobre elas.

Clique com o botão direito num nome da lista para **Afixar na lista** (fica no topo) ou **Remover da lista**. Um nome a cinzento significa que o JeffPub não consegue encontrar esse ficheiro neste momento. Para alterar quantos nomes recentes são mostrados, consulte [Alterar as definições do JeffPub](options).

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista Recentes.

- **Abrir Só de Leitura** abre a publicação apenas para leitura. Pode ver e deslocar-se, mas não pode alterar nada. A barra de título indica "Só de Leitura". Se premir **Guardar**, o JeffPub pede um novo nome, para que o ficheiro que abriu não seja alterado.
- **Abrir como Cópia** abre a publicação sem ficheiro próprio. **Guardar** pede onde a colocar.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria.
- **Abrir e Reparar** destina-se a um ficheiro .jpub danificado, por exemplo um que foi cortado ao ser copiado. O JeffPub lê todas as partes do ficheiro que consegue e deixa de fora as que não consegue ler. A publicação abre-se como uma cópia com o nome "Reparado" seguido do nome do ficheiro, e uma mensagem indica o que ficou de fora. O ficheiro danificado não é alterado.

## Fechar

Clique em **Ficheiro > Fechar** ou prima Ctrl+F4. O JeffPub pergunta se deve guardar as alterações e depois apresenta uma página em branco.

## Informações: propriedades, Marcar como Final, Inspecionar e versões

**Ficheiro > Informações** mostra onde está o ficheiro, com **Mostrar na Pasta** e **Copiar Caminho**. Mostra também dados sobre a sua publicação: páginas, tamanho da página, imagens, palavras e datas. Escreva nas caixas para definir o **Título**, o **Autor**, o **Assunto**, as **Palavras-chave**, as **Categorias**, os **Comentários**, a **Empresa** e o **Gestor**, e clique em **Propriedades Personalizadas** para propriedades próprias. A alteração fica guardada com a publicação, e **Ctrl+Z** a anula. A página abre também as suas informações comerciais, as **Definições de Impressão Comercial** (consulte [Ficheiros PDF e gráficas](pdf)) e o [Verificador de Estrutura](design-checker).

- **Verificar Compatibilidade** lista o que um ficheiro .pub não consegue guardar da sua publicação, sem guardar nada: objetos ocultos, alguns aspetos de listas e tudo o que o escritor de .pub deixa de fora. Quando guarda como ficheiro .pub, o JeffPub mostra a mesma lista **antes** de escrever seja o que for. Escolha **Guardar Mesmo Assim**, **Guardar como .jpub** (o ficheiro .jpub é colocado ao lado, e nada se perde) ou **Cancelar**. Marque **Não voltar a perguntar para este ficheiro** e o JeffPub deixa de perguntar sobre esse ficheiro até fechar o programa.
- **Abrir Sempre Apenas para Leitura** faz com que a publicação abra apenas para leitura de cada vez. A escolha fica guardada com um ficheiro .jpub, e a barra de estado indica-o quando alguém o abre. Para a editar, escolha **Abre Sempre Só para Leitura** na mesma página. É um lembrete e não um bloqueio. Um ficheiro .pub não consegue guardar esta opção (o escritor de .pub não guarda propriedades deste tipo, e o JeffPub não a lê de um ficheiro .pub), por isso guarde primeiro como ficheiro .jpub. Verificar Compatibilidade apresenta-a para um ficheiro .pub.
- **Marcar como Final** guarda a publicação e depois mantém-na apenas para leitura. Quando alguém a abre, a barra de título indica que é só de leitura, e uma nota informa que foi marcada como final. Não é um bloqueio: escolha **Marcado como Final** novamente para a editar. Um ficheiro .pub não consegue guardar esta marca, por isso guarde primeiro como ficheiro .jpub.
- **Inspecionar Publicação** lista o que o ficheiro contém e que talvez não queira partilhar, e remove-o: os comentários e os nomes das pessoas que os escreveram, o autor e outros nomes nas propriedades, as informações comerciais, a lista de impressão em série e os objetos deixados na área de rascunho ao lado das páginas.
- **Remover informações pessoais deste ficheiro ao guardar**, por baixo dos cartões, retira do ficheiro o autor, a pessoa que guardou o ficheiro pela última vez, a empresa e o gestor, de cada vez que guardar, para um ficheiro .jpub ou .pub. É a mesma caixa que está em **Opções**, e é igual em todas as aplicações JeffOffice.
- **Histórico de Versões** mantém cópias com nome da publicação no seu computador. Guarde primeiro a publicação. Clique em **Guardar uma Versão…** e, mais tarde, escolha uma para **Abrir uma Cópia**, **Voltar a Esta Versão** (a publicação tal como está agora fica primeiro guardada como versão) ou **Eliminar**.

Um ficheiro .jpub não tem forma de guardar uma palavra-passe, por isso o JeffPub não tem cartão de palavra-passe. O JeffPub ainda não tem verificação de acessibilidade, por isso a página não tem cartão para essa função.

## Se o Windows não deixar o JeffPub guardar

A Segurança do Windows pode impedir que programas guardem em pastas como Documentos. Nesse caso, o JeffPub mostra uma mensagem com os passos para o permitir. Ou guarde noutra pasta.

## Consulte também

- [Recuperar trabalho não guardado](recover)
- [Trabalhar com ficheiros .pub](pub-files)
- [Criar uma nova publicação](create)
- [Alterar as definições do JeffPub](options)
