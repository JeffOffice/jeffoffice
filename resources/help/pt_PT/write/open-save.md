# Abrir, guardar, imprimir e exportar

<!-- keywords: página web, html, guardar como página web, página web filtrada, correio eletrónico, eml, marcadores, marcadores pdf, shift+f5, última edição, onde parou, modelos incorporados, galeria, carta, relatório, resumo, manuscrito, boletim informativo, abrir, guardar, guardar como, docx, doc, dot, odt, opendocument, rtf, ficheiros antigos, modelo, dotx, recentes, afixar, novo, fechar, imprimir, pdf, exportar, recuperar, recuperação automática, não guardado, falha, ecrã ficheiro, formatos de ficheiro, guardar uma cópia, só de leitura, somente leitura, abrir como cópia, nova janela, cópia de segurança, os meus modelos, guardar como modelo, ott, reparar, abrir e reparar, danificado -->

Clique em **Ficheiro**, no canto superior esquerdo, para ver o ecrã Ficheiro. Tem páginas para Novo, Abrir, Imprimir e Exportar, e comandos para Guardar e Guardar Como. Prima **Esc** para voltar ao seu documento.

## Iniciar um documento novo

Prima **Ctrl+N**, ou clique em **Ficheiro > Novo** e, depois, em **Documento em Branco**. Um novo documento abre numa janela própria. Se a janela em que está estiver vazia e nunca tiver sido guardada, o novo documento ocupa o seu lugar.

## Abrir um documento

Prima **Ctrl+O**, ou clique em **Ficheiro > Abrir**. Clique em **Procurar** para escolher um ficheiro. O JeffWrite abre estes tipos de ficheiro:

- **.docx** é um documento.
- **.dotx** é um modelo. Um modelo abre como um novo documento criado a partir dele, pelo que o modelo se mantém como está.
- **.docm** e **.dotm** são um documento e um modelo que podem conter **macros**, pequenos programas que acompanham um ficheiro. O JeffWrite nunca as executa, e mantém-nas como estavam quando guarda como .docm ou .dotm. Se guardar como .docx ou .odt, as macros são deixadas de fora, e o JeffWrite avisa-o.
- **.txt** é texto simples, apenas as palavras. Ao abrir um, o JeffWrite lê-o de imediato se for ASCII simples ou Unicode. Se não for, o JeffWrite pergunta em que **codificação** o ler e mostra as primeiras linhas, para que veja se as letras estão corretas. Uma codificação é a forma como um ficheiro guarda as letras, como UTF-8 ou Windows 1252. Ao guardar como .txt, escolhe a codificação e os finais de linha, e o ficheiro guarda apenas as palavras: sem tipos de letra, imagens ou linhas de tabela.
- **.html** e **.htm** são páginas web. O JeffWrite lê as palavras, os títulos, o negrito, o itálico, o sublinhado, as cores, as listas e as tabelas. As marcas de lista chegam como marcas escritas, e as imagens e as ligações são deixadas de fora. Nunca transfere nada da Internet, e ignora os scripts. Uma página web nunca é substituída: **Guardar** pede um novo nome e guarda um ficheiro .docx. Para criar uma página web, veja **Guardar como página web** mais abaixo.
- **.doc** e **.dot** são os tipos antigos de documento e de modelo. O JeffWrite lê-os, e guarda o que faz com eles num novo ficheiro .docx.
- **.odt**, **.ott** e **.fodt** são ficheiros OpenDocument. O JeffWrite lê os três. Guarda ficheiros .odt, e guarda o que faz com um ficheiro .ott ou .fodt como um novo ficheiro .odt. Veja [Abrir e guardar ficheiros OpenDocument](open-odt).
- **.rtf** é Rich Text Format. O JeffWrite lê-o e guarda-o. Veja [Abrir ficheiros RTF](open-rtf).

A página **Abrir** também lista os documentos que abriu ou guardou recentemente. Clique num para o abrir. Clique com o botão direito num para o afixar no topo da lista, ou para o retirar da lista.

Se um ficheiro tiver partes que o JeffWrite ainda não consegue editar, como o esquema de uma tabela ou um cabeçalho, mantém-nas. Veja [Bem-vindo ao JeffWrite](start). Se uma parte do ficheiro não puder ser lida de todo, o JeffWrite indica o que deixou de fora. Nesse caso, não substitui esse ficheiro: **Guardar** pede um novo nome, para que o original conserve o que a cópia perdeu.

## Guardar um documento

Prima **Ctrl+S** para guardar. Na primeira vez, o JeffWrite pede um nome. **Guardar Como** (F12) guarda uma cópia com um novo nome. Escolha **Modelo (*.dotx)** para guardar um modelo, **Documento com Macros (*.docm)** para manter as macros, **Texto Simples (*.txt)** para guardar apenas as palavras, ou **Rich Text Format (*.rtf)** para um ficheiro que outros programas consigam ler. Um ficheiro que veio de um tipo de ficheiro mais antigo é sempre guardado com um novo nome, para que o ficheiro antigo nunca seja substituído.

## Guardar uma cópia

Clique em **Ficheiro > Guardar uma Cópia**. Escolha um nome. O JeffWrite grava o documento nesse ficheiro, e continua a trabalhar no documento que tinha aberto. O nome e as alterações por guardar mantêm-se como estavam. Isto é útil quando quer enviar uma versão a alguém e continuar a escrever.

## Manter uma cópia de segurança

Clique em **Ficheiro > Opções** e, na página **Geral**, ative **Criar sempre cópia de segurança**. Sempre que guardar, o JeffWrite copia primeiro o ficheiro antigo para outro chamado "Cópia de segurança de" seguido do nome do ficheiro, na mesma pasta. Se a cópia não puder ser feita, o JeffWrite pergunta se deve guardar sem ela. A resposta é Não, a menos que diga Sim, e o ficheiro mantém-se como estava. Assim, pode voltar à última versão guardada.

## Guardar como modelo

Um modelo é um documento que usa como ponto de partida. Clique em **Ficheiro > Guardar como Modelo** e escreva um nome. O modelo fica em **Os Meus Modelos**, e o documento que está a escrever não é alterado. Em **Guardar Como**, também pode escolher **Modelo (*.dotx)** ou **Modelo de Texto OpenDocument (*.ott)** para guardar um ficheiro de modelo onde preferir.

Para iniciar um documento novo a partir do seu modelo, clique em **Ficheiro > Novo**. Os seus modelos aparecem em **Os Meus Modelos**. Clique num, e um novo documento abre com o texto e os estilos do modelo. Para retirar um modelo da lista, clique com o botão direito nele e escolha **Remover de Os Meus Modelos**.

## Outras formas de abrir um ficheiro

Na página **Abrir**, clique em **Mais Formas de Abrir**. Também pode clicar com o botão direito num nome da lista de recentes. Cada forma pede um ficheiro, ou usa o que clicou:

- **Abrir Só de Leitura** abre o ficheiro apenas para leitura. Pode ver, deslocar, pesquisar e copiar, mas não pode alterar nada. A barra de título mostra "Só de Leitura". Se premir **Guardar**, o JeffWrite pede um novo nome, pelo que o ficheiro que abriu não é alterado.
- **Abrir como Cópia** abre um novo documento com o texto do ficheiro. Não tem ficheiro próprio, por isso **Guardar** pergunta onde o colocar. O ficheiro que abriu nunca é alterado.
- **Abrir numa Nova Janela** abre o ficheiro numa janela própria, mesmo que a janela em que está esteja vazia.
- **Abrir e Reparar** destina-se a um ficheiro .docx danificado, por exemplo um que ficou incompleto ao ser copiado. Veja abaixo.

## Abrir e reparar

Por vezes, um ficheiro .docx fica danificado. Uma cópia pode ter parado a meio, ou um disco pode ter cometido erros. Um ficheiro .docx é um conjunto de partes mais pequenas, como o texto, os estilos e as imagens. **Abrir e Reparar** lê todas as partes que consegue. Uma parte que parou a meio é mantida até ao ponto em que parou. Uma parte que não pode ser lida é deixada de fora. Se a lista de partes ou as ligações entre elas tiverem desaparecido, o JeffWrite volta a criá-las.

O documento reparado abre como uma cópia, com um nome como "Carta reparada". Uma mensagem indica o que foi mantido até à interrupção e o que foi deixado de fora. O ficheiro danificado não é alterado. Guarde a cópia reparada com um nome próprio, e verifique-a antes de confiar nela.

## Recuperar trabalho não guardado

O JeffWrite guarda uma cópia de reserva do trabalho não guardado a cada poucos minutos, num ficheiro na sua própria pasta. Se o programa fechar inesperadamente, oferece essas cópias na próxima vez que o iniciar. Clique em **Ficheiro > Abrir > Recuperar Trabalho Não Guardado** para as ver mais tarde. Pode escolher quais abrir, e guardar cada uma onde quiser.

## Começar a partir de um modelo

**Ficheiro > Novo** mostra o **Documento em Branco** e um conjunto de modelos incluídos no JeffWrite, agrupados por tipo: uma carta, um relatório, um resumo, um manuscrito e um boletim informativo. Cada imagem é a primeira página do documento que obtém. Um **modelo** é um documento já preparado, com os seus estilos, margens e textos de exemplo. Clique num e substitua os textos de exemplo pelos seus. Os modelos que guardou a partir de documentos seus aparecem em **Os Meus Modelos**.

## Retomar onde parou

O JeffWrite recorda onde estava o cursor quando guardou ou fechou um ficheiro. Da próxima vez que abrir o ficheiro, o cursor está nesse ponto. Prima **Shift+F5** para ir aos locais editados por último. Cada toque leva ao anterior: os últimos três locais e, depois, de volta ao primeiro.

## Guardar como .odt

Um ficheiro .odt não consegue conter tudo o que um ficheiro .docx contém. Ao guardar como .odt, o JeffWrite lista primeiro o que se perderia, e permite-lhe guardar um ficheiro .docx em alternativa. Veja [Informações](info).

## Imprimir

Prima **Ctrl+P**, ou clique em **Ficheiro > Imprimir**. Escolha uma impressora, o número de cópias e as páginas, veja cada página na pré-visualização e clique em **Imprimir**. Veja [Imprimir](print).

## Criar um PDF

Clique em **Ficheiro > Exportar > Exportar para PDF**. Escolha as páginas a incluir: todas, a página atual, ou um intervalo como 1-3, 5 (p2s3 e s3 também funcionam aqui). Depois, escolha um nome. O PDF tem o mesmo aspeto em qualquer computador, porque os tipos de letra ficam incluídos no ficheiro. O título, o autor, o assunto e as palavras-chave do documento são transferidos para o PDF. Cada título torna-se um **marcador**, uma entrada na lista que um leitor de PDF mostra ao lado das páginas. Clique num marcador para ir até à respetiva página.

## Guardar como página web

Clique em **Ficheiro > Exportar** e escolha um de dois tipos de página web. Uma **página web** é um ficheiro que pode abrir em qualquer navegador.

- **Guardar como Página Web** cria um único ficheiro .html que contém as suas palavras, o respetivo aspeto e as suas imagens. Pode enviar esse ficheiro sozinho. O aspeto que mantém é o tipo de letra, o tamanho, a cor, o realce, o negrito, o itálico, o sublinhado, o alinhamento e o espaço entre parágrafos.
- **Guardar como Página Web Filtrada** cria um pequeno ficheiro .html apenas com a estrutura: títulos, negrito e itálico, listas, tabelas, ligações e imagens. Não tem tipos de letra, tamanhos nem cores, pelo que pode dar-lhe depois o seu próprio aspeto.

Ambos mantêm os títulos, as listas com marcas e numeradas, as tabelas com células unidas, as ligações e as imagens. O texto oculto, as palavras eliminadas por uma alteração registada, os comentários, os cabeçalhos, os rodapés e as notas são deixados de fora. Uma ligação que iniciaria um programa ou executaria um script é guardada apenas como texto.

## Criar um PDF/A

Na caixa **Exportar para PDF**, marque **PDF/A, para conservação a longo prazo**. Um ficheiro PDF/A inclui todos os tipos de letra que utiliza e segue uma norma criada para conservar ficheiros durante muitos anos. Ainda não tem etiquetas para leitores de ecrã, e não tem palavra-passe.

## Criar um correio eletrónico

Clique em **Ficheiro > Exportar > Criar Correio Eletrónico** para criar um ficheiro de mensagem (.eml). Nada é enviado. Escolhe o que vai na mensagem: o documento, o documento como PDF, ou as páginas apresentadas como imagens na mensagem (as primeiras 30 páginas). Abra o ficheiro no seu programa de correio eletrónico, indique a quem se destina e envie-o.

## Relacionados

- [Bem-vindo ao JeffWrite](start)
- [Imprimir](print)
- [Utilizar o JeffWrite com o teclado](keyboard)
