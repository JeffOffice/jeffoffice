# Abrir e salvar publicações
<!-- keywords: abrir, salvar, salvar como, salvar uma cópia, fechar, arquivos recentes, fixados, tipos de arquivo, jpub, renomear, backup, propriedades, informações, marcar como final, inspecionar, histórico de versões, novo nome, copiar, Ctrl+S, somente leitura, abrir como cópia, nova janela, meus modelos, salvar como modelo, reparar, abrir e reparar, danificado -->

O JeffPub salva o seu trabalho em arquivos .jpub. Ele também abre e salva arquivos .pub. Veja [Trabalhar com arquivos .pub](pub-files).

## Salvar

Pressione Ctrl+S, clique no botão **Salvar** na barra de ferramentas de acesso rápido ou clique em **Arquivo > Salvar**. Na primeira vez, o JeffPub pede um nome e um local. Depois disso, Salvar substitui o arquivo pela sua versão mais recente.

Um asterisco (*) depois do nome na barra de título significa que há alterações que você ainda não salvou. Salve com frequência. Se o JeffPub fechar de repente, veja [Recuperar trabalho não salvo](recover).

Para manter a última cópia salva de um arquivo, ative **Sempre criar cópia de backup**. Cada vez que você salvar, o JeffPub primeiro copia o arquivo antigo para outro com o nome "Backup de" seguido do nome do arquivo, na mesma pasta. Se não for possível criar a cópia, o aplicativo pergunta se deve salvar sem ela. A resposta padrão é Não, a menos que você diga Sim, e o arquivo permanece como estava. Você encontra essa opção em **Arquivo > Opções**, na guia **Geral**. Todos os aplicativos do JeffOffice têm essa opção.

## Salvar com um novo nome

Clique em **Arquivo > Salvar como** ou pressione F12. Digite um nome, escolha **Salvar como tipo** e clique em **Salvar**. As opções são:

- **Publicação do JeffPub (*.jpub)** é a escolha normal.
- **Arquivos de publicação .pub (*.pub)** cria um arquivo .pub.
- **PDF (*.pdf)** cria um PDF e deixa a sua publicação como estava.
- **Modelo do JeffPub (*.jpub)** adiciona a publicação a **Meus Modelos**.

Depois de salvar como .jpub ou .pub, a barra de título mostra o novo nome, e você continua trabalhando no novo arquivo.

## Salvar uma cópia

Clique em **Arquivo > Salvar uma Cópia** e escolha um nome. O JeffPub grava a publicação nesse arquivo, e você continua trabalhando no que tinha aberto. O nome e as alterações não salvas da publicação original permanecem como estavam.

## Abrir

1. Pressione Ctrl+O ou clique em **Arquivo > Abrir**.
2. Clique em **Procurar** para escolher um arquivo. Ou clique em um nome em **Fixados** ou **Publicações Recentes**.

Você também pode arrastar um arquivo .jpub ou .pub para a página. Se a publicação atual tiver alterações não salvas, o JeffPub pergunta sobre elas primeiro.

Clique com o botão direito em um nome da lista para **Fixar na lista** (ele fica no topo) ou **Remover da lista**. Um nome em cinza significa que o JeffPub não consegue encontrar esse arquivo agora. Para mudar quantos nomes recentes aparecem, veja [Alterar as configurações do JeffPub](options).

## Outras formas de abrir um arquivo

Na página **Abrir**, clique em **Mais Formas de Abrir**. Você também pode clicar com o botão direito em um nome da lista de Recentes.

- **Abrir como Somente Leitura** abre a publicação apenas para leitura. Você pode ver e rolar a página, mas não pode alterar nada. A barra de título mostra "Somente Leitura". Se você clicar em **Salvar**, o JeffPub pede um novo nome, para que o arquivo aberto não seja alterado.
- **Abrir como Cópia** abre a publicação sem arquivo próprio. **Salvar** pergunta onde colocá-la.
- **Abrir em Nova Janela** abre o arquivo em uma janela própria.
- **Abrir e Reparar** serve para um arquivo .jpub danificado, por exemplo um que foi cortado ao ser copiado. O JeffPub lê todas as partes do arquivo que consegue e deixa de fora as partes que não consegue ler. A publicação abre como uma cópia chamada "Reparado" seguido do nome, e uma mensagem informa o que ficou de fora. O arquivo danificado não é alterado.

## Fechar

Clique em **Arquivo > Fechar** ou pressione Ctrl+F4. O JeffPub pergunta se deve salvar as alterações e depois mostra uma página em branco.

## Informações: propriedades, Marcar como Final, Inspecionar e versões

**Arquivo > Informações** mostra onde o arquivo está, com **Mostrar na Pasta** e **Copiar Caminho**. Ela exibe dados da sua publicação: páginas, tamanho da página, imagens, palavras e datas. Digite nas caixas para definir **Título**, **Autor**, **Assunto**, **Palavras-chave**, **Categorias**, **Comentários**, **Empresa** e **Gerente**, e clique em **Propriedades Personalizadas** para propriedades próprias. A alteração é salva com a publicação, e **Ctrl+Z** a desfaz. A página também abre suas informações comerciais, as **Configurações de Impressão Comercial** (veja [Arquivos PDF e gráficas](pdf)) e o [Verificador de Design](design-checker).

- **Marcar como Final** salva a publicação e depois a deixa somente para leitura. Quando alguém a abre, a barra de título informa que é somente leitura, e uma nota avisa que ela foi marcada como final. Não é um bloqueio: escolha **Marcada como Final** novamente para editar. Um arquivo .pub não pode guardar essa marca, então salve primeiro como arquivo .jpub.
- **Inspecionar Publicação** lista o que o arquivo contém que você talvez não queira compartilhar, e remove: os comentários e os nomes das pessoas que os escreveram, o autor e outros nomes nas propriedades, as informações comerciais, a lista de mala direta e os objetos guardados na área de rascunho ao lado das páginas.
- **Remover informações pessoais deste arquivo ao salvar**, abaixo dos cartões, retira o autor, a pessoa que salvou o arquivo por último, a empresa e o gerente do arquivo a cada vez que você salvar, para um arquivo .jpub ou .pub. É a mesma caixa das **Opções**, e a mesma em todos os aplicativos do JeffOffice.
- **Histórico de Versões** mantém cópias nomeadas da publicação no seu computador. Salve a publicação primeiro. Clique em **Manter uma Versão…** e, depois, escolha uma para **Abrir uma Cópia** dela, **Voltar a Esta Versão** (a publicação como está agora é guardada antes como uma versão) ou **Excluir**.

Um arquivo .jpub não tem campo para senha, por isso o JeffPub não tem cartão de senha. O JeffPub ainda não tem verificação de acessibilidade, então a página não tem cartão para ela.

## Se o Windows não deixar o JeffPub salvar

A Segurança do Windows pode bloquear programas de salvar em pastas como Documentos. Nesse caso, o JeffPub mostra uma mensagem com os passos para permitir o acesso. Ou salve em outra pasta.

## Relacionados

- [Recuperar trabalho não salvo](recover)
- [Trabalhar com arquivos .pub](pub-files)
- [Criar uma nova publicação](create)
- [Alterar as configurações do JeffPub](options)
